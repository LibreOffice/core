using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Numbering;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;
using Paperless.Vector;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Turns a binary PowerPoint deck into laid-out slides.
/// </summary>
/// <remarks>
/// <para>
/// The rendering counterpart of <see cref="PptContentBuilder"/>, and a second walk over the same
/// records for the reason every family has one: extraction throws away the geometry, fills and
/// sizes rendering needs. What it emits is the same <see cref="LaidOutSlide"/> the PPTX and ODP
/// paths emit, so a slide reaches the drawing sink through one layouter rather than three.
/// </para>
/// <para>
/// <strong>Everything a shape states is in master units of a 576th of an inch</strong>
/// (<c>SvxMSDffManager::SetModel</c>, <c>filter/source/msfilter/msdffimp.cxx:3211</c>) — with the
/// pleasant consequence that a PPT round-trip is <em>exactly</em> reproducible where a PPTX one is
/// not: 576 units divide an inch into 2540 hundredths of a millimetre without remainder, so
/// LibreOffice's own export of a PPT puts a shape stated at 72 pt at 72.000, where the same deck
/// as PPTX comes out at 71.972. The one exception is the text insets, which are in EMUs like
/// everything else in the property table.
/// </para>
/// <para>
/// <strong>The rotation is applied before the flip, which is the opposite of DrawingML.</strong>
/// <c>SvxMSDffManager::ImportShape</c> rotates the object about its bounding box's centre and only
/// then mirrors it about the centre of the <em>resulting</em> snap rectangle
/// (<c>msdffimp.cxx:4899-4918</c>), where <c>oox</c> mirrors first. On a shape that is neither
/// symmetric nor axis-aligned the two orders put it in different corners, so
/// <see cref="ShapeTransform.Place"/> cannot be reused for this and the matrix is composed
/// here.
/// </para>
/// </remarks>
internal sealed class PptSlideLayout
{
    /// <summary>How many master units make an inch.</summary>
    /// <remarks>
    /// 576, which is the whole reason a PPT's coordinates convert exactly: 914400 EMUs divided by
    /// 576 is 1587.5, and any anchor a writer emits is a whole number of units.
    /// </remarks>
    public const int MasterUnitsPerInch = 576;

    /// <summary>How deep a group nest is followed before it is abandoned.</summary>
    private const int MaxGroupDepth = 32;

    /// <summary>The default text insets, a quarter and an eighth of a centimetre in EMUs.</summary>
    /// <remarks>
    /// <c>svdfppt.cxx:853-856</c>: 25 × 3600 across and 13 × 3600 down, which is 7.09 pt and
    /// 3.69 pt. Not DrawingML's 91440/45720 — the two differ by a tenth of a point, which is
    /// exactly the tolerance a placement comparison is held to, so taking the OOXML defaults here
    /// puts every unstated text box on the boundary of failing.
    /// </remarks>
    private const int DefaultInsetAcross = 25 * 3600;
    private const int DefaultInsetDown = 13 * 3600;

    private readonly DffRecordBuffer _stream;
    private readonly PptPersistDirectory _persist;
    private readonly List<Diagnostic> _diagnostics;
    private readonly SlideFonts _fonts;
    private readonly EscherDrawingReader _escher;
    private readonly byte[] _pictures;

    private readonly Dictionary<uint, PptStyleSheet> _stylesByMaster = [];
    private readonly Dictionary<uint, PptColourScheme> _schemesByMaster = [];
    private readonly Dictionary<uint, Paint?> _backgroundsByMaster = [];
    private readonly Dictionary<int, PptPicture> _decoded = [];

    private PptStyleSheet? _defaultStyles;
    private PptFontTable _fontTable = PptFontTable.Empty;
    private Dictionary<int, EscherBlip>? _blips;

    /// <param name="stream">The <c>PowerPoint Document</c> stream.</param>
    /// <param name="persist">The persist directory, which says which version of each object is current.</param>
    /// <param name="fonts">The font stack layout measures with.</param>
    /// <param name="diagnostics">Where anything unreadable is recorded.</param>
    /// <param name="pictures">
    /// The compound file's <c>Pictures</c> stream, which is this format's blip delay stream — a
    /// PPT's <c>msofbtBSE</c> entries carry a <c>foDelay</c> into it and hold no bytes themselves.
    /// Empty for a deck with no pictures, and for a caller that has none to give: the deck then
    /// draws every frame empty, which is what it did before this stream was passed at all.
    /// </param>
    public PptSlideLayout(
        DffRecordBuffer stream,
        PptPersistDirectory persist,
        SlideFonts fonts,
        List<Diagnostic> diagnostics,
        byte[]? pictures = null)
    {
        _stream = stream;
        _persist = persist;
        _fonts = fonts;
        _diagnostics = diagnostics;
        _escher = new EscherDrawingReader(stream, diagnostics);
        _pictures = pictures ?? [];
    }

    /// <summary>Lays every slide out, in presentation order.</summary>
    public List<LaidOutSlide> Layout()
    {
        List<LaidOutSlide> slides = [];
        if (PptPages.Read(_stream, _persist, _diagnostics) is not { } pages) return slides;

        DocSize size = SlideSize(pages);
        _fontTable = PptFontTable.Read(_stream, pages.Environment);
        _blips = ReadBlips(pages);
        ReadMasters(pages);

        for (int index = 0; index < pages.Slides.Count; index++)
        {
            PptPageEntry entry = pages.Slides[index];
            DffRecordHeader? container = PptPages.Resolve(
                _stream, _persist, entry, PptRecordTypes.Slide, _diagnostics);

            slides.Add(container is { } page
                ? LayoutSlide(page, entry, index, size)
                : new LaidOutSlide { Index = index, Size = size });
        }

        return slides;
    }

    /// <summary>
    /// The deck's slide size, from the first eight bytes of the <c>DocumentAtom</c>.
    /// </summary>
    /// <remarks>
    /// Two signed 32-bit master-unit lengths, before the notes size and the zoom ratio
    /// (<c>ReadPptDocumentAtom</c>, <c>svdfppt.cxx:257</c>). A deck with no atom gets the 10 × 7.5
    /// inch screen page PowerPoint defaults to, which is what a blank presentation is.
    /// </remarks>
    private DocSize SlideSize(PptPages pages)
    {
        if (_stream.FirstChild(pages.Document, PptRecordTypes.DocumentAtom) is { } atom)
        {
            ReadOnlySpan<byte> content = _stream.Content(atom);
            if (content.Length >= 8)
            {
                int width = unchecked((int)DffRecordBuffer.ReadUInt32(content));
                int height = unchecked((int)DffRecordBuffer.ReadUInt32(content[4..]));
                if (width > 0 && height > 0) return new DocSize(Units(width), Units(height));
            }
        }

        return new DocSize(Length.FromInches(10), Length.FromInches(7.5));
    }

    /// <summary>Converts a master-unit length to a <see cref="Length"/>.</summary>
    private static Length Units(long units)
        => Length.FromEmu(units * Length.EmuPerInch / MasterUnitsPerInch);

    /// <summary>
    /// The deck's blip store: every picture, indexed by the <c>pib</c> a shape carries.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A PPT keeps its drawing group in a <c>PPDrawingGroup</c> record inside the document
    /// container, and the group's first child is the <c>OfficeArtBStoreContainer</c> — the same
    /// structure a DOC keeps at <c>fcDggInfo</c>, which is why one reader serves both. The
    /// entries here are always the <c>foDelay</c> form: PowerPoint writes the picture bytes into
    /// the compound file's <c>Pictures</c> stream and leaves the entry at its bare thirty-six
    /// bytes. LibreOffice hands <c>SdrPowerPointImport</c> exactly that stream as its BLIP stream
    /// (<c>sd/source/filter/ppt/pptin.cxx:216</c>, <c>maPictureStream</c>).
    /// </para>
    /// <para>
    /// <strong>Why the omission was expensive.</strong> A metafile blip in a PPT is zlib-deflated
    /// behind its <c>OfficeArtMetafileHeader</c>, so a deck's pasted tables, charts and org
    /// diagrams are invisible to any search of the file for their own text — which is what makes
    /// "the reference draws words our reader cannot even find" look like a text bug rather than a
    /// missing picture. Twelve of the thirteen worst text losses in the slides corpus were this.
    /// </para>
    /// </remarks>
    private Dictionary<int, EscherBlip> ReadBlips(PptPages pages)
    {
        if (_stream.FirstChild(pages.Document, PptRecordTypes.DrawingGroup) is not { } group)
            return [];

        foreach (DffRecordHeader child in _stream.Children(group))
        {
            if (child.Type != EscherRecordTypes.DrawingGroupContainer) continue;

            return EscherBlips.Read(_stream, child, _pictures, []);
        }

        return [];
    }

    /// <summary>
    /// The picture a shape's <c>pib</c> names, decoded once per store entry.
    /// </summary>
    /// <remarks>
    /// <c>pib</c> is one-based and zero means "no picture", so the lookup and the emptiness test
    /// are the same question. Which of raster and vector a blip is comes from its bytes rather
    /// than from its record type, for the reason <see cref="Ooxml.PptxSlideLayout"/> gives at
    /// length: a producer writing a genuine EMF into a <c>WMF</c> blip is ordinary, and the
    /// decoder registry is the only thing that knows what it can read.
    /// </remarks>
    private PptPicture PictureOf(EscherShape shape)
    {
        uint pib = shape.Properties.Value(EscherPropertyIds.Picture);
        if (pib == 0 || _blips is null) return default;
        if (_decoded.TryGetValue((int)pib, out PptPicture cached)) return cached;
        if (!_blips.TryGetValue((int)pib, out EscherBlip blip)) return default;

        PptPicture picture = default;
        if (!blip.Bytes.IsEmpty)
        {
            ReadOnlyMemory<byte> bytes = blip.Bytes;
            picture = VectorImages.For(bytes.Span) is not null
                ? new PptPicture(null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)))
                : new PptPicture(RasterImage.Encoded(bytes, MediaType(blip.RecordType)), null);
        }

        _decoded[(int)pib] = picture;
        return picture;
    }

    /// <summary>
    /// What a blip record type says its bytes are, for a backend choosing a decoder.
    /// </summary>
    /// <remarks>
    /// The record type is the only honest label a binary Office file gives a picture — there is
    /// no file name and no content type — and it is a hint rather than a fact: a decoder still
    /// sniffs. Null for the types that are not rasters, which never reach here.
    /// </remarks>
    private static string? MediaType(ushort recordType) => recordType switch
    {
        0xF01D or 0xF02A => "image/jpeg",
        0xF01E => "image/png",
        0xF01F => "image/bmp",
        0xF029 => "image/tiff",
        _ => null,
    };

    /// <summary>One blip store entry's picture: a raster, or a vector decoded when something draws it.</summary>
    private readonly record struct PptPicture(RasterImage? Raster, Lazy<VectorImage>? Vector)
    {
        /// <summary>True when the entry held nothing drawable.</summary>
        public bool IsEmpty => Raster is null && Vector is null;
    }

    /// <summary>
    /// Reads what each master supplies to the slides under it: styles, colours and background.
    /// </summary>
    /// <remarks>
    /// A title master states another master's slide id and carries no styles of its own, so the
    /// derived ones are resolved in a second pass — the first cannot, because the list may write
    /// a title master before the main master it points at.
    /// </remarks>
    private void ReadMasters(PptPages pages)
    {
        Dictionary<uint, uint> derived = [];

        foreach (PptPageEntry entry in pages.Masters)
        {
            if (_persist.Resolve(entry.PersistId) is not { } offset) continue;
            if (!_stream.TryReadHeader(offset, out DffRecordHeader header)) continue;
            if (header.Type != PptRecordTypes.MainMaster) continue;

            if (SchemeOf(header) is { } scheme) _schemesByMaster[entry.SlideId] = scheme;

            uint parent = PptPages.MasterIdOf(_stream, header) ?? 0;
            if (parent != 0)
            {
                derived[entry.SlideId] = parent;
                continue;
            }

            PptStyleSheet sheet = PptStyleSheet.Read(_stream, header, pages.Environment);
            _stylesByMaster[entry.SlideId] = sheet;
            _defaultStyles = sheet;

            _backgroundsByMaster[entry.SlideId] = BackgroundOf(
                header, _schemesByMaster.GetValueOrDefault(entry.SlideId, PptColourScheme.Default));
        }

        foreach ((uint child, uint parent) in derived)
        {
            if (_stylesByMaster.TryGetValue(parent, out PptStyleSheet? sheet))
            {
                _stylesByMaster[child] = sheet;
            }
            else if (_defaultStyles is { } fallback)
            {
                _stylesByMaster[child] = fallback;
            }

            if (_backgroundsByMaster.TryGetValue(parent, out Paint? background))
            {
                _backgroundsByMaster[child] = background;
            }
        }
    }

    private PptColourScheme? SchemeOf(DffRecordHeader page)
        => _stream.FirstChild(page, PptRecordTypes.ColorSchemeAtom) is { } atom
            ? PptColourScheme.Read(_stream.Content(atom))
            : null;

    /// <summary>
    /// The page's own colour scheme, or its master's when the slide follows it.
    /// </summary>
    /// <remarks>
    /// Bit 1 of the <c>SlideAtom</c>'s flags word means "follow the master's colour scheme"
    /// (<c>SdrPowerPointImport::GetColorFromPalette</c>, <c>svdfppt.cxx:2566-2604</c>). LibreOffice
    /// still reads the slide's own atom and then throws it away when the bit is set, which is what
    /// makes a recoloured master reach every slide under it.
    /// </remarks>
    private PptColourScheme SchemeFor(DffRecordHeader page, uint masterId, ushort flags)
    {
        if ((flags & 0x0002) == 0 && SchemeOf(page) is { } own) return own;
        if (_schemesByMaster.TryGetValue(masterId, out PptColourScheme? master)) return master;
        return SchemeOf(page) ?? PptColourScheme.Default;
    }

    private LaidOutSlide LayoutSlide(
        DffRecordHeader page, PptPageEntry entry, int index, DocSize size)
    {
        (uint masterId, ushort flags) = SlideAtom(page);
        PptColourScheme scheme = SchemeFor(page, masterId, flags);
        PptStyleSheet? styles = _stylesByMaster.GetValueOrDefault(masterId) ?? _defaultStyles;

        bool hidden = false;
        foreach (DffRecordHeader record in _stream.Children(page))
        {
            if (record.Type == PptRecordTypes.SlideShowSlideInfoAtom)
            {
                hidden |= IsHidden(_stream.Content(record));
            }
        }

        List<PlacedShape> shapes = [];
        Paint? background = BackgroundOf(page, scheme)
                            ?? _backgroundsByMaster.GetValueOrDefault(masterId)
                            ?? Paint.Solid(Colour.White);

        Context context = new(entry, scheme, styles);

        if (_stream.FirstChild(page, PptRecordTypes.Drawing) is { } drawing
            && _stream.FirstChild(drawing, EscherRecordTypes.DrawingContainer) is { } container)
        {
            foreach (EscherShape shape in _escher.ReadDrawing(container))
            {
                Add(shape, context, AffineTransform.Identity, shapes, depth: 0);
            }
        }

        return new LaidOutSlide
        {
            Index = index,
            Size = size,
            IsHidden = hidden,
            Background = background,
            Shapes = shapes,
        };
    }

    /// <summary>The master id and flags of a page's <c>SlideAtom</c>.</summary>
    private (uint MasterId, ushort Flags) SlideAtom(DffRecordHeader page)
    {
        if (_stream.FirstChild(page, PptRecordTypes.SlideAtom) is not { } atom) return (0, 0);

        ReadOnlySpan<byte> content = _stream.Content(atom);
        uint master = content.Length >= 16 ? DffRecordBuffer.ReadUInt32(content[12..]) : 0;
        ushort flags = content.Length >= 22 ? DffRecordBuffer.ReadUInt16(content[20..]) : (ushort)0;
        return (master, flags);
    }

    /// <summary>
    /// Whether a slide's show information marks it as skipped.
    /// </summary>
    /// <remarks>
    /// The flag lives in the middle of a transition record: four bytes of advance time, four of a
    /// sound reference, then single bytes of direction and transition type, and only then the
    /// flags word whose bit 2 means "do not show" (<c>sd/source/filter/ppt/pptin.cxx:1825</c>).
    /// </remarks>
    private static bool IsHidden(ReadOnlySpan<byte> content)
        => content.Length >= 12 && (DffRecordBuffer.ReadUInt16(content[10..]) & 0x0004) != 0;

    /// <summary>
    /// The page's background fill, or null when it states none.
    /// </summary>
    /// <remarks>
    /// The background is a shape rather than a property: a <c>SpContainer</c> whose flags carry
    /// <see cref="EscherShapeAttributes.Background"/>, sitting beside the patriarch group rather
    /// than inside it. LibreOffice reads its <c>fillColor</c> with a default of white
    /// (<c>svdfppt.cxx:3055-3060</c>), which is why a deck that says nothing is on paper.
    /// </remarks>
    private Paint? BackgroundOf(DffRecordHeader page, PptColourScheme scheme)
    {
        if (_stream.FirstChild(page, PptRecordTypes.Drawing) is not { } drawing) return null;
        if (_stream.FirstChild(drawing, EscherRecordTypes.DrawingContainer) is not { } container)
            return null;

        foreach (EscherShape shape in _escher.ReadDrawing(container))
        {
            if (!shape.IsBackground) continue;
            return Paint.Solid(
                PptColour.Resolve(shape.Properties.Value(EscherPropertyIds.FillColour, 0xFFFFFF), scheme)
                ?? Colour.White);
        }

        return null;
    }

    /// <summary>What a page supplies to every shape on it.</summary>
    private sealed record Context(
        PptPageEntry Entry, PptColourScheme Scheme, PptStyleSheet? Styles);

    private void Add(
        EscherShape shape,
        Context context,
        AffineTransform space,
        List<PlacedShape> shapes,
        int depth)
    {
        if (shape.IsBackground || shape.IsDeleted) return;
        if (shape.Properties.Boolean(EscherPropertyIds.Hidden)) return;

        if (shape.IsGroup)
        {
            if (depth >= MaxGroupDepth) return;

            AffineTransform inner = GroupSpace(shape, space);
            foreach (EscherShape child in shape.Children)
            {
                Add(child, context, inner, shapes, depth + 1);
            }

            return;
        }

        if (Place(shape, context, space) is { } placed) shapes.Add(placed);
    }

    /// <summary>
    /// The matrix taking a group's children onto the slide.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group states two rectangles, and <strong>the one that says what its children are measured
    /// in is not the one the record is named after</strong>. LibreOffice ignores
    /// <c>msofbtSpgr</c>'s bounds and uses the <em>union of the children's own anchors</em>
    /// (<c>SvxMSDffManager::GetGlobalChildAnchor</c>, <c>msdffimp.cxx:4972</c>), mapping that onto
    /// the group's client anchor. The two usually agree to within a unit — on
    /// <c>ppt-features.ppt</c>'s group they differ by one, which is a fortieth of a point — but
    /// only one of them is what the reference renders, and it is the union.
    /// </para>
    /// <para>
    /// A group with no children to union, or whose union is degenerate, maps one to one rather
    /// than dividing by zero.
    /// </para>
    /// </remarks>
    private AffineTransform GroupSpace(EscherShape group, AffineTransform space)
    {
        if (AnchorOf(group) is not { } target) return space;

        EscherRectangle? union = null;
        foreach (EscherShape child in group.Children)
        {
            if (AnchorOf(child) is not { } anchor) continue;
            union = union is { } so_far ? Union(so_far, anchor) : anchor;
        }

        if (union is not { } source || source.Width == 0 || source.Height == 0) return space;

        double scaleX = (double)target.Width / source.Width;
        double scaleY = (double)target.Height / source.Height;

        // Children are stated in the source space; take them to the origin, scale, and put them
        // where the group sits. Composed onto whatever space the group itself was in, so a nested
        // group's factors cumulate.
        AffineTransform map = AffineTransform.Concat(
            AffineTransform.Translation(-Units(source.Left).Emu, -Units(source.Top).Emu),
            AffineTransform.Scale(scaleX, scaleY));

        return AffineTransform.Concat(
            AffineTransform.Concat(
                map, AffineTransform.Translation(Units(target.Left).Emu, Units(target.Top).Emu)),
            space);
    }

    private static EscherRectangle Union(EscherRectangle a, EscherRectangle b) => new(
        Math.Min(a.Left, b.Left), Math.Min(a.Top, b.Top),
        Math.Max(a.Right, b.Right), Math.Max(a.Bottom, b.Bottom));

    /// <summary>
    /// A shape's rectangle in whatever space it is stated in.
    /// </summary>
    /// <remarks>
    /// A group member states a <c>msofbtChildAnchor</c> in its group's space; anything else states
    /// a <c>msofbtClientAnchor</c> in the page's. PowerPoint's client anchor is the odd one:
    /// eight bytes of <em>signed 16-bit</em> values in the order top, left, right, bottom — the
    /// order really is that, and LibreOffice's own comment on the line reading it says "the order
    /// of coordinates is a bit strange" (<c>svdfppt.cxx:586</c>). Newer files write a sixteen-byte
    /// form in the ordinary left, top, right, bottom order instead.
    /// </remarks>
    private EscherRectangle? AnchorOf(EscherShape shape)
    {
        if (shape.ClientAnchor is { } anchor)
        {
            ReadOnlySpan<byte> content = _stream.Content(anchor);

            if (content.Length >= 16)
            {
                return new EscherRectangle(
                    unchecked((int)DffRecordBuffer.ReadUInt32(content)),
                    unchecked((int)DffRecordBuffer.ReadUInt32(content[4..])),
                    unchecked((int)DffRecordBuffer.ReadUInt32(content[8..])),
                    unchecked((int)DffRecordBuffer.ReadUInt32(content[12..])));
            }

            if (content.Length >= 8)
            {
                short top = unchecked((short)DffRecordBuffer.ReadUInt16(content));
                short left = unchecked((short)DffRecordBuffer.ReadUInt16(content[2..]));
                short right = unchecked((short)DffRecordBuffer.ReadUInt16(content[4..]));
                short bottom = unchecked((short)DffRecordBuffer.ReadUInt16(content[6..]));
                return new EscherRectangle(left, top, right, bottom);
            }
        }

        return shape.ChildAnchor;
    }

    private PlacedShape? Place(EscherShape shape, Context context, AffineTransform space)
    {
        if (AnchorOf(shape) is not { } anchor) return null;

        DocRect local = new(
            Units(anchor.Left), Units(anchor.Top), Units(anchor.Width), Units(anchor.Height));

        if (local.Width <= Length.Zero && local.Height <= Length.Zero) return null;

        double rotation = Rotation(shape);
        local = Uprighted(local, rotation);

        AffineTransform placement = Placement(
            local,
            rotation,
            (shape.Flags & EscherShapeAttributes.FlipHorizontal) != 0,
            (shape.Flags & EscherShapeAttributes.FlipVertical) != 0,
            space);

        string? preset = PptShapeGeometry.PresetOf(shape.ShapeType);
        int? adjustment = shape.Properties.Has(PptShapeGeometry.AdjustValue)
            ? PptShapeGeometry.Adjustment(
                shape.ShapeType, shape.Properties.SignedValue(PptShapeGeometry.AdjustValue))
            : null;

        // A shape's own vertex array outranks its type, because LibreOffice's exporter writes one
        // on nearly every shape and names no preset at all; falling through to the type would draw
        // a bounding rectangle for a triangle it had the exact path for.
        GraphicsPath outline =
            (PptCustomGeometry.Has(shape.Properties)
                ? PptCustomGeometry.Outline(shape.Properties, local.Size)
                : null)
            ?? SlidePresetGeometry.Outline(preset, local.Size, Guides(adjustment));

        DocRect bounds = ShapeTransform.PlacedBounds(placement, local.Size);

        return new PlacedShape
        {
            Name = shape.Name,
            Outline = ShapeTransform.Apply(placement, outline),
            Bounds = bounds,
            Fill = Fill(shape, context.Scheme),
            Line = Line(shape, context.Scheme),
            Picture = Picture(shape, bounds),
            Text = Text(shape, context, local, preset, adjustment, placement),
        };
    }

    /// <summary>
    /// The picture a shape draws, in the rectangle the shape occupies.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Any shape may carry one, not only a picture frame.</strong> Escher has no separate
    /// picture element: a <c>pib</c> is a property like a fill colour, so a rounded rectangle with
    /// a photograph in it is one shape with both. <c>SvxMSDffManager::ImportShape</c> reads it the
    /// same way — the graphic is fetched whenever the property is present
    /// (<c>msdffimp.cxx:4693</c>) — which is why this is asked of every shape rather than of a
    /// type.
    /// </para>
    /// <para>
    /// The destination is the shape's placed rectangle, with no crop applied.
    /// <c>cropFromTop</c> and its three siblings are recorded in the TODO rather than
    /// approximated; a cropped picture drawn whole is the right picture in the right place at the
    /// wrong scale, where dropping it is a hole.
    /// </para>
    /// </remarks>
    private PlacedPicture? Picture(EscherShape shape, DocRect bounds)
    {
        PptPicture picture = PictureOf(shape);
        return picture.IsEmpty
            ? null
            : new PlacedPicture(picture.Raster, bounds) { Vector = picture.Vector };
    }

    /// <summary>
    /// The rotation in radians, clockwise, from the property table's 16.16 fixed-point degrees.
    /// </summary>
    private static double Rotation(EscherShape shape)
    {
        int raw = shape.Properties.SignedValue(EscherPropertyIds.Rotation);
        return raw == 0 ? 0 : raw / 65536.0 * Math.PI / 180.0;
    }

    /// <summary>
    /// The rectangle a near-quarter-turn rotation is measured in, which is the stated one turned
    /// on its side.
    /// </summary>
    /// <remarks>
    /// <strong>The trap this format keeps for a reader who has done DrawingML first.</strong> For
    /// an angle between 45° and 135° — or between 225° and 315° — the anchor a PPT states is the
    /// rectangle the shape occupies <em>after</em> rotating, not before, so the width and the
    /// height have to be swapped about the centre before the rotation is applied
    /// (<c>msdffimp.cxx:4356-4368</c>). Skip it and a shape at 90° comes out with its two
    /// dimensions the wrong way round, which on a square is invisible and on a title bar is a
    /// shape lying across the slide.
    /// </remarks>
    private static DocRect Uprighted(DocRect local, double rotation)
    {
        double degrees = rotation * 180.0 / Math.PI;
        degrees -= Math.Floor(degrees / 360.0) * 360.0;

        bool quarterTurned = (degrees > 45 && degrees <= 135) || (degrees > 225 && degrees <= 315);
        if (!quarterTurned) return local;

        Length halfWidth = Length.FromEmu(local.Width.Emu / 2);
        Length halfHeight = Length.FromEmu(local.Height.Emu / 2);

        return new DocRect(
            local.X + halfWidth - halfHeight,
            local.Y + halfHeight - halfWidth,
            local.Height,
            local.Width);
    }

    /// <summary>
    /// The matrix taking a shape's own box onto the slide: rotate, <em>then</em> mirror.
    /// </summary>
    /// <remarks>
    /// Both about the shape's centre, which is where the two operations commute for the centre
    /// itself and differ for everything else. The order is the drawing layer's rather than
    /// DrawingML's; see the note on the class.
    /// </remarks>
    private static AffineTransform Placement(
        DocRect bounds,
        double rotation,
        bool flipHorizontal,
        bool flipVertical,
        AffineTransform space)
    {
        double halfWidth = bounds.Width.Emu / 2.0;
        double halfHeight = bounds.Height.Emu / 2.0;

        AffineTransform transform = AffineTransform.Translation(-halfWidth, -halfHeight);

        if (rotation != 0)
        {
            transform = AffineTransform.Concat(transform, AffineTransform.Rotation(rotation));
        }

        if (flipHorizontal || flipVertical)
        {
            transform = AffineTransform.Concat(
                transform, AffineTransform.Scale(flipHorizontal ? -1 : 1, flipVertical ? -1 : 1));
        }

        transform = AffineTransform.Concat(
            transform,
            AffineTransform.Translation(bounds.X.Emu + halfWidth, bounds.Y.Emu + halfHeight));

        return AffineTransform.Concat(transform, space);
    }

    /// <summary>
    /// A shape's fill, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <c>fFilled</c> is a boolean packed into property 447, and a shape that does not
    /// <em>state</em> it takes its type's default rather than the packed bit
    /// (<c>DffPropertyReader::ApplyFillAttributes</c>, <c>msdffimp.cxx:1320</c>). Only a solid fill
    /// is resolved; a gradient or a picture is left unpainted rather than approximated by one of
    /// its stops, which is the same rule the other two paths follow.
    /// </remarks>
    private static Paint? Fill(EscherShape shape, PptColourScheme scheme)
    {
        bool filled = shape.Properties.StatesBoolean(EscherPropertyIds.Filled)
            ? shape.Properties.Boolean(EscherPropertyIds.Filled)
            : PptShapeGeometry.IsFilledByDefault(shape.ShapeType);

        if (!filled) return null;
        if (shape.Properties.Value(PptShapeGeometry.FillType, 0) != PptShapeGeometry.SolidFill)
            return null;

        return PptColour.Resolve(
            shape.Properties.Value(EscherPropertyIds.FillColour, 0xFFFFFF), scheme) is { } colour
            ? Paint.Solid(colour)
            : null;
    }

    /// <summary>
    /// A shape's outline, or null when it has none.
    /// </summary>
    /// <remarks>
    /// The width defaults to 9525 EMUs — three quarters of a point — which is what the drawing
    /// layer draws for a shape that states a line and no thickness (<c>msdffimp.cxx:916</c>).
    /// </remarks>
    private static Stroke? Line(EscherShape shape, PptColourScheme scheme)
    {
        bool lined = shape.Properties.StatesBoolean(EscherPropertyIds.Lined)
            ? shape.Properties.Boolean(EscherPropertyIds.Lined)
            : PptShapeGeometry.IsStrokedByDefault(shape.ShapeType);

        if (!lined) return null;

        if (PptColour.Resolve(
                shape.Properties.Value(EscherPropertyIds.LineColour, 0), scheme) is not { } colour)
        {
            return null;
        }

        return new Stroke(
            Paint.Solid(colour),
            Length.FromEmu(shape.Properties.Value(EscherPropertyIds.LineWidth, 9525)),
            Cap(shape.Properties.Value(PptShapeGeometry.LineEndCap, 0)),
            Join(shape.Properties.Value(PptShapeGeometry.LineJoin, PptShapeGeometry.MiterJoin)));
    }

    private static LineCap Cap(uint cap) => cap switch
    {
        1 => LineCap.Square,
        2 => LineCap.Round,
        _ => LineCap.Butt,
    };

    private static LineJoin Join(uint join) => join switch
    {
        0 => LineJoin.Bevel,
        1 => LineJoin.Miter,
        _ => LineJoin.Round,
    };

    private PlacedText? Text(
        EscherShape shape,
        Context context,
        DocRect local,
        string? preset,
        int? adjustment,
        AffineTransform placement)
    {
        if (TextOf(shape, context) is not { } run) return null;
        if (PptTextBody.Build(run, context.Styles, context.Scheme, _fontTable, Insets(shape),
                              Anchor(shape), Wraps(shape)) is not { } body)
        {
            return null;
        }

        DocRect rectangle = SlidePresetGeometry.TextRectangle(preset, local.Size, Guides(adjustment));

        // Upright means "axis-aligned and not mirrored", not "the identity". A group scales its
        // children, and on this format's commonest group — a client anchor one master unit wider
        // than the union of its children's — the factor is 1.00025, which is neither one nor a
        // rotation. Treating that as rotated puts the shape's text into the shape's own coordinate
        // space behind a matrix, where it is correct on the page and unreadable to every tool that
        // compares a pen position against a reference renderer's. Mapping the rectangle instead is
        // also what LibreOffice does: a group scale resizes the shape and the text is laid out
        // again inside it at its own size, rather than being stretched.
        bool upright = placement is { B: 0, C: 0, A: > 0, D: > 0 };
        DocRect area = upright
            ? new DocRect(
                ShapeTransform.Apply(placement, rectangle.Origin),
                new DocSize(
                    Length.FromEmu((long)Math.Round(rectangle.Width.Emu * placement.A)),
                    Length.FromEmu((long)Math.Round(rectangle.Height.Emu * placement.D))))
            : rectangle;

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, _fonts);
        return runs.Count == 0 ? null : new PlacedText(runs, upright ? AffineTransform.Identity : placement);
    }

    private static Margins Insets(EscherShape shape) => new(
        Length.FromEmu(shape.Properties.SignedValue(EscherPropertyIds.TextInsetLeft, DefaultInsetAcross)),
        Length.FromEmu(shape.Properties.SignedValue(EscherPropertyIds.TextInsetTop, DefaultInsetDown)),
        Length.FromEmu(shape.Properties.SignedValue(EscherPropertyIds.TextInsetRight, DefaultInsetAcross)),
        Length.FromEmu(shape.Properties.SignedValue(EscherPropertyIds.TextInsetBottom, DefaultInsetDown)));

    /// <summary>Where the text block sits vertically, from <c>anchorText</c>.</summary>
    /// <remarks>
    /// Ten values, of which the "centered" and "baseline" halves differ only in how the block is
    /// justified <em>across</em> the shape; vertically they collapse to the three
    /// <see cref="TextAnchor"/> positions (<c>svdfppt.cxx:927-950</c>).
    /// </remarks>
    private static TextAnchor Anchor(EscherShape shape)
        => shape.Properties.Value(EscherPropertyIds.TextAnchor, 0) switch
        {
            1 or 4 => TextAnchor.Middle,
            2 or 5 or 7 or 9 => TextAnchor.Bottom,
            _ => TextAnchor.Top,
        };

    private static bool Wraps(EscherShape shape)
        => shape.Properties.Value(EscherPropertyIds.WrapText, 0) != PptShapeGeometry.WrapNone;

    /// <summary>
    /// A shape's text, whether it holds the characters itself or refers to the slide list.
    /// </summary>
    private PptTextRun? TextOf(EscherShape shape, Context context)
    {
        if (shape.ClientTextbox is not { } textbox) return null;

        int start = textbox.ContentStart;
        int end = _stream.EndOf(textbox);

        foreach (DffRecordHeader record in _stream.Range(start, end))
        {
            if (record.Type != PptRecordTypes.OutlineTextRefAtom) continue;

            uint reference = DffRecordBuffer.ReadUInt32(_stream.Content(record));
            return OutlineText(context.Entry, reference);
        }

        return PptTextReader.Read(_stream, start, end);
    }

    /// <summary>
    /// The <paramref name="reference"/>th text run of a slide's entry in the document's slide
    /// list.
    /// </summary>
    private PptTextRun? OutlineText(PptPageEntry entry, uint reference)
    {
        int matches = 0;
        int start = -1;

        foreach (DffRecordHeader record in _stream.Range(entry.TextStart, entry.TextEnd))
        {
            if (record.Type == PptRecordTypes.SlidePersistAtom) break;
            if (record.Type != PptRecordTypes.TextHeaderAtom) continue;

            if (start >= 0) return PptTextReader.Read(_stream, start, record.Position);
            if (matches++ == reference) start = record.Position;
        }

        return start >= 0 ? PptTextReader.Read(_stream, start, entry.TextEnd) : null;
    }

    /// <summary>
    /// Escher's single adjust value as the named guides the preset evaluator asks for.
    /// </summary>
    /// <remarks>
    /// The two vocabularies count the same thing in the same units — a fraction of the shape's
    /// 21600-unit box — but disagree about its name. Escher has one unnamed `adjustValue`
    /// property per shape; a DrawingML preset declares its own guide names, and the evaluator
    /// looks each up by the name the definition states. The convention across
    /// `presetShapeDefinitions.xml` is `adj` for a shape with one handle and `adj1` for the
    /// first of several, and no preset declares both, so offering the value under each name
    /// hands it to whichever one the definition happens to use rather than guessing.
    /// </remarks>
    private static Dictionary<string, double>? Guides(int? adjustment)
        => adjustment is not { } value
            ? null
            : new Dictionary<string, double>(StringComparer.Ordinal)
            {
                ["adj"] = value,
                ["adj1"] = value,
            };

}
