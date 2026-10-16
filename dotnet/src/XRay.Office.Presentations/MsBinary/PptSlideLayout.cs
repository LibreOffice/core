using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbering;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.MsBinary.Records;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using XRay.Office.Vector;

namespace XRay.Office.Presentations.MsBinary;

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
    private readonly byte[] _summary;

    private readonly Dictionary<uint, PptStyleSheet> _stylesByMaster = [];
    private readonly Dictionary<uint, PptColourScheme> _schemesByMaster = [];
    private readonly Dictionary<uint, uint> _masterParents = [];
    private readonly Dictionary<uint, DffRecordHeader> _pagesByMaster = [];
    private readonly Dictionary<uint, PptPageEntry> _entriesByMaster = [];
    private readonly Dictionary<int, PptPicture> _decoded = [];

    private readonly Dictionary<uint, EscherPropertyTable> _shapePropertiesById = [];

    private PptStyleSheet? _defaultStyles;
    private PptFontTable _fontTable = PptFontTable.Empty;

    /// <summary>
    /// The identifiers of the hyperlinks the deck declares, which is what makes a text range's
    /// <c>InteractiveInfo</c> a field. See <see cref="PptHyperlinks"/>.
    /// </summary>
    private IReadOnlySet<uint> _hyperlinks = new HashSet<uint>();
    private Dictionary<int, EscherBlip>? _blips;

    /// <summary>The document's picture-bullet store, read once for the deck.</summary>
    private PptBulletPictures _bulletPictures = PptBulletPictures.None;
    private PptHeadersFooters _deckHeadersFooters = PptHeadersFooters.None;
    private bool _titlePlaceholdersOmitted;

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
    /// <param name="summary">
    /// The compound file's <c>\u0005DocumentSummaryInformation</c> stream, whose
    /// <c>_PID_HLINKS</c> blob caps how many of the deck's hyperlink identifiers resolve — see
    /// <see cref="PptHyperlinkBlob"/>. Empty for a caller that has none to give.
    /// </param>
    public PptSlideLayout(
        DffRecordBuffer stream,
        PptPersistDirectory persist,
        SlideFonts fonts,
        List<Diagnostic> diagnostics,
        byte[]? pictures = null,
        byte[]? summary = null)
    {
        _summary = summary ?? [];
        _stream = stream;
        _persist = persist;
        _fonts = fonts;
        _diagnostics = diagnostics;
        _escher = new EscherDrawingReader(stream, diagnostics);
        _pictures = pictures ?? [];

        // Through the field rather than through its value: the collection lives in the Environment
        // container and is not read until `Layout` runs, which is still before any text is measured.
        _fonts.DeclaredPitches = name => _fontTable.PitchOf(name);
    }

    /// <summary>Lays every slide out, in presentation order.</summary>
    public List<LaidOutSlide> Layout()
    {
        List<LaidOutSlide> slides = [];
        if (PptPages.Read(_stream, _persist, _diagnostics) is not { } pages) return slides;

        DocSize size = SlideSize(pages);
        _fontTable = PptFontTable.Read(_stream, pages.Environment);
        _hyperlinks = PptHyperlinks.Read(_stream, pages.Document, _summary);
        _blips = ReadBlips(pages);
        _bulletPictures = PptBulletPictures.Read(_stream, pages.Document);
        _deckHeadersFooters = DeckHeadersFooters(pages);
        _titlePlaceholdersOmitted = TitlePlaceholdersOmitted(pages);
        ReadMasters(pages);
        IndexShapes(pages);

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

    /// <summary>
    /// Whether the deck suppresses the running placeholders on its title slides.
    /// </summary>
    /// <remarks>
    /// Byte 37 of the <c>DocumentAtom</c>, past the two page sizes, the zoom ratio, the two
    /// master persist ids, the first page number, the page format and the embedded-fonts flag
    /// (<c>ReadPptDocumentAtom</c>, <c>svdfppt.cxx:257-290</c>). It is the reason a deck's title
    /// slide carries no footer while every slide after it does
    /// (<c>sd/source/filter/ppt/pptin.cxx:1456-1461</c>).
    /// </remarks>
    private bool TitlePlaceholdersOmitted(PptPages pages)
    {
        if (_stream.FirstChild(pages.Document, PptRecordTypes.DocumentAtom) is not { } atom)
            return false;

        ReadOnlySpan<byte> content = _stream.Content(atom);
        return content.Length >= 38 && content[37] != 0;
    }

    /// <summary>
    /// The deck-wide running-placeholder settings its slide masters start from.
    /// </summary>
    /// <remarks>
    /// The document container carries one <c>HeadersFooters</c> per master kind, told apart by
    /// the header instance: 3 for the slide masters and 4 for the notes master
    /// (<c>svdfppt.cxx:1636-1653</c>). Only the slide one is read here; a notes page is not
    /// rendered.
    /// </remarks>
    private PptHeadersFooters DeckHeadersFooters(PptPages pages)
    {
        PptHeadersFooters result = PptHeadersFooters.None;

        foreach (DffRecordHeader child in _stream.Children(pages.Document))
        {
            if (child.Type != PptRecordTypes.HeadersFooters || child.Instance != 3) continue;
            result = PptHeadersFooters.Read(_stream, child, result);
        }

        return result;
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
        => PictureAt(shape.Properties.Value(EscherPropertyIds.Picture));

    /// <summary>
    /// The raster a shape's <c>fillBlip</c> names, for a pattern, texture or picture fill.
    /// </summary>
    /// <remarks>
    /// A fill blip and a displayed picture are different properties naming the same store, so
    /// this is the same lookup under a different key. Rasters only: a
    /// <see cref="BitmapPaint"/> carries pixels, so a metafile fill blip has nowhere to go and
    /// is left unpainted rather than rasterised here.
    /// </remarks>
    private RasterImage? FillPictureOf(EscherShape shape)
        => PictureAt(shape.Properties.Value(PptFills.FillBlip)).Raster;

    private PptPicture PictureAt(uint pib)
    {
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
    /// <para>
    /// A title master states another master's slide id and carries no styles of its own, so the
    /// derived ones are resolved in a second pass — the first cannot, because the list may write
    /// a title master before the main master it points at.
    /// </para>
    /// <para>
    /// <strong>A title master is written as a <c>Slide</c> record, not a <c>MainMaster</c>
    /// one</strong>, and requiring the latter drops it — with it the deck's title slides lose
    /// every decoration the master carries, since nothing then answers for that master id.
    /// LibreOffice never asks: it takes whatever the persist id resolves to and looks inside for
    /// a <c>SlideAtom</c> (<c>svdfppt.cxx:1583-1586</c>), which both kinds carry and a handout
    /// does not. Both are accepted here for the same reason, and the master list is already
    /// separate from the notes list, so nothing else can arrive through it.
    /// </para>
    /// </remarks>
    private void ReadMasters(PptPages pages)
    {
        Dictionary<uint, uint> derived = [];

        foreach (PptPageEntry entry in pages.Masters)
        {
            if (_persist.Resolve(entry.PersistId) is not { } offset) continue;
            if (!_stream.TryReadHeader(offset, out DffRecordHeader header)) continue;
            if (header.Type is not (PptRecordTypes.MainMaster or PptRecordTypes.Slide)) continue;

            _pagesByMaster[entry.SlideId] = header;
            _entriesByMaster[entry.SlideId] = entry;

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
        }

        foreach ((uint child, uint parent) in derived)
        {
            _masterParents[child] = parent;

            if (_stylesByMaster.TryGetValue(parent, out PptStyleSheet? sheet))
            {
                _stylesByMaster[child] = sheet;
            }
            else if (_defaultStyles is { } fallback)
            {
                _stylesByMaster[child] = fallback;
            }
        }
    }

    /// <summary>
    /// The background a master supplies, resolved against the scheme the <em>slide</em> is being
    /// drawn under.
    /// </summary>
    /// <remarks>
    /// Resolved per slide rather than cached with the master because the master's background shape
    /// states a scheme <em>slot</em>, and which colours that slot names is the current page's
    /// question: LibreOffice re-imports the master's background object into the slide's
    /// <c>ProcessData</c> (<c>svdfppt.cxx:2846-2852</c>), so it resolves under whatever
    /// <c>GetColorFromPalette</c> answers for the slide. A deck that recolours one slide and still
    /// follows the master's background is the case that separates the two.
    /// </remarks>
    private Paint? MasterBackground(uint masterId, PptColourScheme scheme, DocSize size)
    {
        uint id = masterId;

        // A title master can follow its own master's background, so the chain is walked rather
        // than looked up once (svdfppt.cxx:2833-2841, which walks the same chain and guards
        // against a file whose masters point at each other).
        for (int hop = 0; hop <= MaxMasterChain; hop++)
        {
            if (!_pagesByMaster.TryGetValue(id, out DffRecordHeader master)) return null;
            if (BackgroundOf(master, scheme, size) is { } paint) return paint;
            if (!_masterParents.TryGetValue(id, out uint parent) || parent == id) return null;
            id = parent;
        }

        return null;
    }

    /// <summary>How far a master-follows-master chain is walked before it is abandoned.</summary>
    private const int MaxMasterChain = 8;

    /// <summary>
    /// Indexes every shape in the deck by identifier, then lets the drawing reader inherit
    /// through it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>SvxMSDffManager::SeekToShape</c> (<c>msdffimp.cxx:6270</c>) without the cluster
    /// table: that walks a shape id back through <c>maFidcls</c> to the drawing that owns it and
    /// scans that drawing, and since a shape id is unique across the file, one dictionary answers
    /// the same question. Groups are descended into, because the C++ loop falls through a group
    /// container's header rather than stepping over it and so reaches nested shapes too.
    /// </para>
    /// <para>
    /// <strong>Built before the resolver is attached, so the tables it holds have inherited
    /// nothing.</strong> A master's placeholder may itself name a master — a title master
    /// deriving from the main one — and indexing an already-merged table would compound the two
    /// silently. The C++ has the property for free: it re-reads the master shape's own
    /// <c>msofbtOPT</c> record from the stream, so it can only ever see one level.
    /// </para>
    /// </remarks>
    private void IndexShapes(PptPages pages)
    {
        foreach (PptPageEntry entry in pages.Masters.Concat(pages.Slides).Concat(pages.Notes))
        {
            if (_persist.Resolve(entry.PersistId) is not { } offset) continue;
            if (!_stream.TryReadHeader(offset, out DffRecordHeader page)) continue;
            if (_stream.FirstChild(page, PptRecordTypes.Drawing) is not { } drawing) continue;
            if (_stream.FirstChild(drawing, EscherRecordTypes.DrawingContainer) is not { } container)
                continue;

            foreach (EscherShape shape in _escher.ReadDrawing(container))
            {
                foreach (EscherShape each in shape.SelfAndDescendants())
                {
                    if (each.ShapeId != 0 && each.Properties.Count > 0)
                    {
                        _shapePropertiesById.TryAdd(each.ShapeId, each.Properties);
                    }
                }
            }
        }

        _escher.MasterShapeProperties = id
            => _shapePropertiesById.GetValueOrDefault(id);
    }

    private PptColourScheme? SchemeOf(DffRecordHeader page)
        => PptColourScheme.OfPage(_stream, page);

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
        if ((flags & FollowMasterScheme) == 0 && SchemeOf(page) is { } own) return own;
        if (_schemesByMaster.TryGetValue(masterId, out PptColourScheme? master)) return master;
        return SchemeOf(page) ?? PptColourScheme.Default;
    }

    /// <summary>
    /// The scheme the master's running placeholders have to be re-resolved under, or null when the
    /// master's own scheme is right for them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>HeaderFooterEntry::NeedToImportInstance</c>
    /// (<c>filter/source/msfilter/svdfppt.cxx:3125</c>). The date, header, footer and slide number
    /// are shapes on the master, so drawing them means drawing master shapes — but a slide that
    /// recolours itself has to draw them in <em>its own</em> colours, or a deck whose title slide
    /// inverts the scheme shows a dark footer on a dark background.
    /// </para>
    /// <para>
    /// LibreOffice implements it by re-importing each placeholder from the master's recorded file
    /// offset (<c>HeaderFooterOfs</c>, set at <c>svdfppt.cxx:772</c> for exactly these four ids)
    /// into the <em>slide's</em> <c>ProcessData</c>, so the same Escher shape resolves its scheme
    /// colours against the slide. Here the master's shapes are walked anyway, so the equivalent is
    /// to hand those four a different scheme rather than to read them twice.
    /// </para>
    /// <para>
    /// The condition is both halves of the reference's: the slide must not be following the
    /// master's scheme, <em>and</em> the two schemes must actually differ. A slide that states its
    /// own atom identical to its master's — which is what PowerPoint writes on most pages — is
    /// not a recolour and must keep drawing under the master's.
    /// </para>
    /// </remarks>
    private PptColourScheme? RecolouredRunningPlaceholders(
        uint masterId, ushort flags, PptColourScheme slideScheme)
    {
        if ((flags & FollowMasterScheme) != 0) return null;
        if (!_schemesByMaster.TryGetValue(masterId, out PptColourScheme? master)) return null;

        return slideScheme.SameColoursAs(master) ? null : slideScheme;
    }

    private LaidOutSlide LayoutSlide(
        DffRecordHeader page, PptPageEntry entry, int index, DocSize size)
    {
        (uint masterId, ushort flags, int layout) = SlideAtom(page);
        PptColourScheme scheme = SchemeFor(page, masterId, flags);
        PptStyleSheet? styles = _stylesByMaster.GetValueOrDefault(masterId) ?? _defaultStyles;

        bool hidden = false;
        PptHeadersFooters runningPlaceholders = _deckHeadersFooters;

        foreach (DffRecordHeader record in _stream.Children(page))
        {
            if (record.Type == PptRecordTypes.SlideShowSlideInfoAtom)
            {
                hidden |= IsHidden(_stream.Content(record));
            }
            else if (record.Type == PptRecordTypes.HeadersFooters)
            {
                runningPlaceholders = PptHeadersFooters.Read(_stream, record, runningPlaceholders);
            }
        }

        // A deck that omits its title placeholders shows none of the four on a title slide, no
        // matter what the atom says (pptin.cxx:1456-1461).
        if (_titlePlaceholdersOmitted && layout == TitleSlideLayout)
        {
            runningPlaceholders = PptHeadersFooters.None;
        }

        List<PlacedShape> shapes = [];

        // Bit 2 decides which background shape is the page's, and a slide that sets it carries one
        // of its own regardless — PowerPoint writes a background shape on every page whether or not
        // it is used. Preferring the slide's own would draw the wrong one on most decks.
        Paint? background = (flags & FollowMasterBackground) != 0
            ? MasterBackground(masterId, scheme, size) ?? BackgroundOf(page, scheme, size)
            : BackgroundOf(page, scheme, size) ?? MasterBackground(masterId, scheme, size);

        background ??= Paint.Solid(Colour.White);

        PptFieldValues fields = FieldsFor(index, runningPlaceholders);
        Context context = new(entry, scheme, styles, fields);

        if ((flags & FollowMasterObjects) != 0)
        {
            AddMasterShapes(
                masterId, runningPlaceholders, fields,
                RecolouredRunningPlaceholders(masterId, flags, scheme), shapes);
        }

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

    /// <summary>
    /// Bit 0 of a <c>SlideAtom</c>'s flags: draw the master's shapes under this slide.
    /// </summary>
    /// <remarks>
    /// Clearing it is how PowerPoint makes a title slide that keeps its own copies of the
    /// decorations instead of the master's; Impress implements it by taking the background-objects
    /// layer out of the page's visible set (<c>sd/source/filter/ppt/pptin.cxx:1548-1557</c>).
    /// </remarks>
    private const ushort FollowMasterObjects = 0x0001;

    /// <summary>
    /// Bit 1 of a <c>SlideAtom</c>'s flags: resolve scheme colours against the master's atom
    /// rather than the page's own (<c>svdfppt.cxx:2566-2604</c>).
    /// </summary>
    private const ushort FollowMasterScheme = 0x0002;

    /// <summary>
    /// Bit 2 of a <c>SlideAtom</c>'s flags: take the master's background shape rather than the
    /// page's own (<c>svdfppt.cxx:2826</c>, "follow master background?").
    /// </summary>
    private const ushort FollowMasterBackground = 0x0004;

    /// <summary>The <c>SlideLayoutAtom</c> geometry meaning "title slide".</summary>
    private const int TitleSlideLayout = 0;

    /// <summary>The layout, master id and flags of a page's <c>SlideAtom</c>.</summary>
    /// <remarks>
    /// The layout geometry comes first, then its eight placeholder ids — twelve bytes in all —
    /// and only then the master and notes ids and the flags word.
    /// </remarks>
    private (uint MasterId, ushort Flags, int Layout) SlideAtom(DffRecordHeader page)
    {
        if (_stream.FirstChild(page, PptRecordTypes.SlideAtom) is not { } atom) return (0, 0, -1);

        ReadOnlySpan<byte> content = _stream.Content(atom);
        int layout = content.Length >= 4
            ? unchecked((int)DffRecordBuffer.ReadUInt32(content))
            : -1;
        uint master = content.Length >= 16 ? DffRecordBuffer.ReadUInt32(content[12..]) : 0;
        ushort flags = content.Length >= 22 ? DffRecordBuffer.ReadUInt16(content[20..]) : (ushort)0;
        return (master, flags, layout);
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
    /// <para>
    /// The background is a shape rather than a property: a <c>SpContainer</c> whose flags carry
    /// <see cref="EscherShapeAttributes.Background"/>, sitting beside the patriarch group rather
    /// than inside it. LibreOffice runs it through the ordinary <c>ApplyAttributes</c> and only
    /// then overwrites the fill <em>colour</em> (<c>svdfppt.cxx:3055-3060</c>) — so a shaded or
    /// picture background keeps its shading, and it is the solid case that falls back to white.
    /// </para>
    /// <para>
    /// The page rectangle is the box a gradient is measured in and the rectangle a picture is
    /// stretched across, so it has to be passed in: the shape's own anchor says nothing, and
    /// LibreOffice substitutes a fixed rectangle for the same reason.
    /// </para>
    /// </remarks>
    private Paint? BackgroundOf(DffRecordHeader page, PptColourScheme scheme, DocSize size)
    {
        if (_stream.FirstChild(page, PptRecordTypes.Drawing) is not { } drawing) return null;
        if (_stream.FirstChild(drawing, EscherRecordTypes.DrawingContainer) is not { } container)
            return null;

        foreach (EscherShape shape in _escher.ReadDrawing(container))
        {
            if (!shape.IsBackground) continue;

            return PptFills.Resolve(
                       shape.Properties,
                       filled: true,
                       scheme,
                       new DocRect(DocPoint.Origin, size),
                       AffineTransform.Identity,
                       FillPictureOf(shape))
                   ?? Paint.Solid(
                       PptColour.Resolve(
                           shape.Properties.Value(EscherPropertyIds.FillColour, 0xFFFFFF),
                           scheme, shape.Properties, EscherPropertyIds.FillColour)
                       ?? Colour.White);
        }

        return null;
    }

    /// <summary>What a page supplies to every shape on it.</summary>
    private sealed record Context(
        PptPageEntry Entry,
        PptColourScheme Scheme,
        PptStyleSheet? Styles,
        PptFieldValues Fields)
    {
        /// <summary>
        /// Whether the shape being placed is a member of a group the reference turns into a
        /// table. See <see cref="IsTableGroup"/>: a cell's text never autofits.
        /// </summary>
        public bool InTable { get; init; }

        /// <summary>
        /// The rows the reference re-derives for the table group the shape sits in, or null when
        /// it sits in none or nothing moves. See <see cref="PptTableRows"/>.
        /// </summary>
        public PptTableRows? Rows { get; init; }
    }

    /// <summary>
    /// What the running fields resolve to on a page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The slide number is the page's position, one-based, which is what Impress's page field
    /// renders. Header, footer and date come from the page's own running-placeholder settings.
    /// </para>
    /// <para>
    /// <strong>An automatic date is left unresolved.</strong> LibreOffice inserts a live date
    /// field for it (<c>PPTFieldEntry::SetDateTime</c>, <c>svdfppt.cxx:6449</c>), so the reference
    /// rendering of such a deck says whatever day it was made on. Substituting today's date here
    /// would agree with a reference taken today and disagree with one taken yesterday, which makes
    /// every stored comparison a clock. The marker is dropped instead, so the page carries no
    /// stray asterisk either way.
    /// </para>
    /// </remarks>
    private static PptFieldValues FieldsFor(int index, PptHeadersFooters runningPlaceholders)
        => new(
            SlideNumber: (index + 1).ToString(System.Globalization.CultureInfo.InvariantCulture),
            Date: runningPlaceholders.DateIsAutomatic ? "" : runningPlaceholders.Date ?? "",
            Header: runningPlaceholders.Header ?? "",
            Footer: runningPlaceholders.Footer ?? "");

    /// <summary>
    /// Draws what a slide inherits from its master, under the slide's own shapes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A PPT master page is drawn whole, not merged into the slide's placeholders.</strong>
    /// Impress imports every shape of a <c>MainMaster</c> except the patriarch onto the master page
    /// and puts it on the background-objects layer
    /// (<c>sd/source/filter/ppt/pptin.cxx:838-848</c>); the slide then shows that layer unless its
    /// <c>SlideAtom</c> clears <see cref="FollowMasterObjects"/>. So a logo, a strapline and a rule
    /// on the master reach every slide under it without any of them appearing in the slide's own
    /// drawing — which is why a deck whose master carries its branding loses a fifth of its words
    /// when this walk is missing, with nothing in the slide's records to suggest where they went.
    /// </para>
    /// <para>
    /// Two kinds of master shape are held back. The master's own title, body, subtitle and notes
    /// prompts — placeholder ids 1 to 6 — are presentation objects rather than background objects
    /// and Impress never draws a master's presentation object while a slide is shown
    /// (<see cref="PptPlaceholders.IsMasterPrompt"/>). The date, header, footer and slide-number
    /// placeholders are drawn only where the page's own running-placeholder settings ask for them.
    /// </para>
    /// <para>
    /// The master's colour scheme and style sheet are used rather than the slide's, because the
    /// shapes belong to the master page: a slide that recolours itself recolours its own shapes,
    /// not the branding it inherits.
    /// </para>
    /// </remarks>
    private void AddMasterShapes(
        uint masterId,
        PptHeadersFooters runningPlaceholders,
        PptFieldValues fields,
        PptColourScheme? recoloured,
        List<PlacedShape> shapes)
    {
        if (!_pagesByMaster.TryGetValue(masterId, out DffRecordHeader master)) return;
        if (_stream.FirstChild(master, PptRecordTypes.Drawing) is not { } drawing) return;
        if (_stream.FirstChild(drawing, EscherRecordTypes.DrawingContainer) is not { } container)
            return;

        Context context = new(
            _entriesByMaster.GetValueOrDefault(masterId),
            _schemesByMaster.GetValueOrDefault(masterId, PptColourScheme.Default),
            _stylesByMaster.GetValueOrDefault(masterId) ?? _defaultStyles,
            fields);

        Context slideColours = recoloured is null ? context : context with { Scheme = recoloured };

        foreach (EscherShape shape in _escher.ReadDrawing(container))
        {
            int placeholder = PlaceholderOf(shape);
            if (PptPlaceholders.IsMasterPrompt(placeholder)) continue;
            if (!runningPlaceholders.Shows(placeholder)) continue;

            // Only the four running placeholders move to the slide's scheme; the master's own
            // decorations stay in the master's, which is what re-importing exactly the shapes
            // HeaderFooterOfs recorded amounts to.
            Context effective = PptPlaceholders.IsRunning(placeholder) ? slideColours : context;
            Add(shape, effective, AffineTransform.Identity, shapes, depth: 0);
        }
    }

    /// <summary>
    /// The placeholder a shape's client data declares, or <see cref="PptPlaceholders.None"/>.
    /// </summary>
    /// <remarks>
    /// The id is one byte behind a four-byte placement id
    /// (<c>ReadPptOEPlaceholderAtom</c>, <c>svdfppt.cxx:490</c>). An ordinary shape has no client
    /// data at all, so the absent record and the zero id mean the same thing.
    /// </remarks>
    private int PlaceholderOf(EscherShape shape)
    {
        if (shape.ClientData is not { } data) return PptPlaceholders.None;

        foreach (DffRecordHeader record in _stream.Children(data))
        {
            if (record.Type != PptRecordTypes.PlaceholderAtom) continue;

            ReadOnlySpan<byte> content = _stream.Content(record);
            return content.Length >= 5 ? content[4] : PptPlaceholders.None;
        }

        return PptPlaceholders.None;
    }

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

            // A group the reference replaces by a table hands its members' text to cells, and a
            // cell keeps none of the item set that would have autofitted it. The rows are then
            // re-derived from what those cells need, which the members' own anchors only bound.
            Context inside = context.InTable || !IsTableGroup(shape)
                ? context
                : context with { InTable = true, Rows = TableRows(shape, context, inner) };
            foreach (EscherShape child in shape.Children)
            {
                Add(child, inside, inner, shapes, depth + 1);
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

        // A table member is drawn where the table's own layout puts it, not where the group's
        // rectangle sat: the reference discards the group and lays the cells out again.
        if (context.Rows is { } rows) local = rows.Remap(local);

        if (local.Width <= Length.Zero && local.Height <= Length.Zero) return null;

        double rotation = Rotation(shape);
        local = Uprighted(local, rotation);

        string? preset = PptShapeGeometry.PresetOf(shape.ShapeType);

        // The stated value is kept rather than the converted one because the conversion is not
        // always a constant: a trapezoid's inset is a fraction of the shape's *width* here and a
        // fraction of its shortest side there, so the answer moves when the shape is refitted.
        int? stated = shape.Properties.Has(PptShapeGeometry.AdjustValue)
            ? shape.Properties.SignedValue(PptShapeGeometry.AdjustValue)
            : null;
        double? adjustment = Adjustment(shape.ShapeType, stated, local.Size);

        // The shape resizes itself around its own text before anything else is measured against
        // it: the fill, the outline, the picture and the text all sit in the *fitted* rectangle.
        PptShapeText? text = TextIn(shape, context);
        if (text is not null)
        {
            local = Fitted(shape, text, local, preset, adjustment);
            adjustment = Adjustment(shape.ShapeType, stated, local.Size);
        }

        AffineTransform placement = Placement(
            local,
            rotation,
            (shape.Flags & EscherShapeAttributes.FlipHorizontal) != 0,
            (shape.Flags & EscherShapeAttributes.FlipVertical) != 0,
            space);

        // An Escher WordArt is not a shape with text on it: `EnhancedCustomShapeEngine::render2`
        // replaces the whole custom shape with the curves `EnhancedCustomShapeFontWork::CreateFontWork`
        // builds, so the box, its geometry and its shadow are not drawn at all. The fill survives
        // because `CreateSdrObjectFromParagraphOutlines` copies the shape's own item set onto the
        // path object -- see `PptFontwork`.
        if (PptFontwork.Outline(shape, local.Size, _fonts) is { } warped)
        {
            return new PlacedShape
            {
                Name = shape.Name,
                Outline = ShapeTransform.Apply(placement, warped),
                Bounds = ShapeTransform.PlacedBounds(placement, local.Size),
                Fill = Fill(shape, context.Scheme, local, placement),
            };
        }

        // A shape's own vertex array outranks its type, because LibreOffice's exporter writes one
        // on nearly every shape and names no preset at all; falling through to the type would draw
        // a bounding rectangle for a triangle it had the exact path for.
        GraphicsPath? own = PptCustomGeometry.Has(shape.Properties)
            ? PptCustomGeometry.Outline(shape.Properties, local.Size)
            : null;

        CustomShapeGeometry.Geometry resolved = own is null
            ? Mirrored(SlidePresetGeometry.Of(preset, local.Size, Guides(adjustment)), shape, local.Size)
            : new CustomShapeGeometry.Geometry(
                own, new DocRect(Length.Zero, Length.Zero, local.Size.Width, local.Size.Height));

        GraphicsPath outline = ShapeTransform.Apply(placement, resolved.Outline);

        // What is filled, what is stroked and what is shaded, all of which a subpath states for
        // itself. A shape drawing its own vertex array reports none, so it is painted whole,
        // exactly as before.
        PaintedGeometry painted = SlidePresetGeometry.Painted(resolved, placement, outline);

        DocRect bounds = ShapeTransform.PlacedBounds(placement, local.Size);

        return new PlacedShape
        {
            Name = shape.Name,
            Outline = outline,
            FillOutline = painted.Fill,
            StrokeOutline = painted.Stroke,
            ShadedParts = painted.ShadedParts,
            Bounds = bounds,
            Fill = Fill(shape, context.Scheme, local, placement),
            Line = Line(shape, context.Scheme, context.InTable),
            Picture = Picture(shape, bounds),
            Text = Text(text, local, shape, preset, adjustment, placement),
            Shadow = Shadow(shape, context.Scheme),
        };
    }

    /// <summary>
    /// The drop shadow a shape casts, from its Escher shadow properties.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SvxMSDffManager::ApplyAttributes</c> (<c>filter/source/msfilter/msdffimp.cxx:2643-2733</c>),
    /// with the two defaults that make the difference between a shadow in the right place and one
    /// a third of a millimetre off. A shape that says it casts a shadow and states no offset gets
    /// 35 hundredths of a millimetre in each axis; one whose <c>shadowType</c> is anything but a
    /// plain offset, and which has no non-zero offset, gets 302 instead.
    /// </para>
    /// <para>
    /// The offsets are EMUs here and hundredths of a millimetre there — <c>ScaleEmu</c>
    /// (<c>msdffimp.cxx:3194</c>) is a division by 360 and a map factor that is 1 for a
    /// presentation — so the property values go straight into a <see cref="Length"/> and the two
    /// defaults are the ones that need converting back.
    /// </para>
    /// <para>
    /// Not implemented: the <c>bCheckShadow</c> branch that turns a shadow off for a shape with
    /// neither fill nor outline. It is <c>static bool bCheckShadow(false)</c> in the reference —
    /// dead code kept for an easy revert, with the comment "#i124477# Found no reason not to set
    /// shadow, esp. since it is applied to evtl. existing text" — so following it would differ
    /// from the binary that draws the references.
    /// </para>
    /// </remarks>
    private static SlideShadow? Shadow(EscherShape shape, PptColourScheme scheme)
    {
        EscherPropertyTable properties = shape.Properties;
        if (!properties.Has(EscherPropertyIds.Shadowed | 31)) return null;
        if (!properties.Boolean(EscherPropertyIds.Shadowed)) return null;

        int x = properties.SignedValue(EscherPropertyIds.ShadowOffsetX, int.MinValue);
        int y = properties.SignedValue(EscherPropertyIds.ShadowOffsetY, int.MinValue);
        bool statesOffset = x != int.MinValue || y != int.MinValue;

        if (x == int.MinValue) x = HundredthsOfAMillimetre(35);
        if (y == int.MinValue) y = HundredthsOfAMillimetre(35);

        bool nonZeroOffset = x > 0 || y > 0;
        if (properties.Has(EscherPropertyIds.ShadowType)
            && properties.Value(EscherPropertyIds.ShadowType) != 0
            && !(statesOffset && nonZeroOffset))
        {
            x = y = HundredthsOfAMillimetre(302);
        }

        Colour colour = PptColour.Resolve(
            properties.Value(EscherPropertyIds.ShadowColour, DefaultShadowColour),
            scheme, properties, EscherPropertyIds.ShadowColour)
            ?? new Colour(0x80, 0x80, 0x80);

        // 16.16 fixed point, so 0x10000 is opaque and a missing property is too.
        double opacity = Math.Clamp(
            properties.Value(EscherPropertyIds.ShadowOpacity, 0x10000) / 65536.0, 0, 1);

        return new SlideShadow(
            Length.FromEmu(x), Length.FromEmu(y), colour.WithAlpha(255), opacity);
    }

    /// <summary>The default shadow colour, <c>0x00808080</c> in the format's blue-green-red order.</summary>
    private const uint DefaultShadowColour = 0x00808080;

    /// <summary>A length the reference states in hundredths of a millimetre, in EMUs.</summary>
    private static int HundredthsOfAMillimetre(int value) => value * 360;

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
    /// The destination is the shape's placed rectangle grown by whatever <c>cropFromTop</c> and
    /// its three siblings throw away, so the surviving part of the picture is what fills the
    /// frame — <see cref="EscherPicture.Cropped"/>. The clip that makes it look cropped is the
    /// shape's outline, which <see cref="Layout.SlideDrawing"/> applies to every picture anyway.
    /// Measured on <c>Thailand17.ppt</c> page 22: an anchor of 655.63 × 528.25 pt under crops of
    /// 8% left, 2.667% right and 10% bottom becomes 733.92 × 586.97, which is the reference's own
    /// destination to 0.03 pt in three independent coordinates.
    /// </para>
    /// </remarks>
    private PlacedPicture? Picture(EscherShape shape, DocRect bounds)
    {
        PptPicture picture = PictureOf(shape);
        if (picture.IsEmpty) return null;

        // Attached here rather than in PictureAt, because the knockout is a property of the
        // *shape* and that cache is keyed by pib: two shapes may display one stored blip with
        // different colours knocked out, and folding it into the cache would give the second
        // shape the first shape's transparency. Vectors are excluded for the reason
        // ColourKnockout records — the reference applies it only to a Bitmap graphic.
        RasterImage? raster = picture.Raster;
        if (raster is not null
            && picture.Vector is null
            && EscherPicture.TransparentColour(shape.Properties) is { } knockout)
        {
            raster = raster with { Knockout = knockout };
        }

        return new PlacedPicture(raster, EscherPicture.Cropped(shape.Properties, bounds))
            {
                Vector = picture.Vector,

                // Every picture in a PPT is inline by construction: a blip lives in the Escher
                // blip store, zlib-deflated, and never as a stream of its own.
                IsInline = true,
            };
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
    /// (<c>DffPropertyReader::ApplyFillAttributes</c>, <c>msdffimp.cxx:1320</c>). What the fill
    /// then <em>is</em> — solid, shaded or a bitmap — is <see cref="PptFills"/>'s question, and
    /// the same one a page background asks.
    /// </remarks>
    private Paint? Fill(
        EscherShape shape, PptColourScheme scheme, DocRect local, AffineTransform placement)
    {
        bool filled = shape.Properties.StatesBoolean(EscherPropertyIds.Filled)
            ? shape.Properties.Boolean(EscherPropertyIds.Filled)
            : PptShapeGeometry.IsFilledByDefault(shape.ShapeType);

        // The same split the text path makes: an upright shape's fill is stated in slide
        // coordinates outright, so a shading's numbers land in a backend's output directly
        // comparable with a reference renderer's; a rotated one keeps its own box and travels
        // with the matrix, which a GradientPaint carries and a backend applies.
        bool upright = IsUpright(placement);
        DocRect box = upright
            ? new DocRect(ShapeTransform.Apply(placement, DocPoint.Origin), local.Size)
            : new DocRect(DocPoint.Origin, local.Size);

        return PptFills.Resolve(
            shape.Properties,
            filled,
            scheme,
            box,
            upright ? AffineTransform.Identity : placement,
            FillPictureOf(shape));
    }

    /// <summary>True when a placement is a pure translation, so a fill needs no matrix.</summary>
    private static bool IsUpright(AffineTransform transform)
        => transform.A == 1 && transform.B == 0 && transform.C == 0 && transform.D == 1;

    /// <summary>
    /// A shape's outline, or null when it has none.
    /// </summary>
    /// <remarks>
    /// The width defaults to 9525 EMUs — three quarters of a point — which is what the drawing
    /// layer draws for a shape that states a line and no thickness (<c>msdffimp.cxx:916</c>).
    /// </remarks>
    private static Stroke? Line(EscherShape shape, PptColourScheme scheme, bool inTable = false)
    {
        bool lined = shape.Properties.StatesBoolean(EscherPropertyIds.Lined)
            ? shape.Properties.Boolean(EscherPropertyIds.Lined)
            : PptShapeGeometry.IsStrokedByDefault(shape.ShapeType);

        if (!lined) return null;

        if (PptColour.Resolve(
                shape.Properties.Value(EscherPropertyIds.LineColour, 0),
                scheme, shape.Properties, EscherPropertyIds.LineColour) is not { } colour)
        {
            return null;
        }

        uint stated = shape.Properties.Value(EscherPropertyIds.LineWidth, 9525);

        return new Stroke(
            Paint.Solid(colour),
            inTable && IsTableRule(shape)
                ? TableBorderWidth(stated)
                : Length.FromEmu(stated),
            Cap(shape.Properties.Value(PptShapeGeometry.LineEndCap, 0)),
            Join(shape.Properties.Value(PptShapeGeometry.LineJoin, PptShapeGeometry.MiterJoin)));
    }

    /// <summary>
    /// Whether a member of a table group is one of the group's rules rather than one of its
    /// cells — the reference's <c>IsLine</c>.
    /// </summary>
    /// <remarks>
    /// <c>IsLine</c> (<c>svdfppt.cxx</c>:7183) asks for an <c>SdrPathObj</c> that is a line with
    /// two points, and the only shape <c>SvxMSDffManager::ImportShape</c> builds one of is an
    /// unextruded <c>mso_sptLine</c> (<c>msdffimp.cxx</c>:4403-4412). Measured over the corpus's
    /// 51 <c>.ppt</c>, every one of the 1089 members of the 34 table groups is either that or a
    /// rectangle-kind cell.
    /// </remarks>
    private static bool IsTableRule(EscherShape shape)
        => shape.ShapeType == PptShapeGeometry.LineShape
           && (shape.Properties.Value(PptShapeGeometry.ThreeDimensionalFlags, 0)
               & PptShapeGeometry.Extruded) == 0;

    /// <summary>
    /// The width the reference finally strokes a <c>.ppt</c> table's rule at, from the EMUs the
    /// file states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Not the stated width, and on the corpus's own numbers it is well under half of
    /// it.</strong> A rule inside a table group never reaches the page as a line: the reference
    /// turns it into the neighbouring cells' <c>BorderLine2</c> and draws it from there, and the
    /// value is mangled twice on the way.
    /// </para>
    /// <list type="number">
    /// <item><description>
    /// <c>SvxMSDffManager::ScaleEmu</c> (<c>msdffimp.cxx</c>:3193, applied at :1048) divides the
    /// EMUs by 360 into hundredths of a millimetre, <em>truncating</em>: one point, 12700 EMU, is
    /// 35 and not 35.28.
    /// </description></item>
    /// <item><description>
    /// <c>ApplyCellLineAttributes</c> (<c>svdfppt.cxx</c>:7513-7531) sets
    /// <c>LineWidth = max(1, XATTR_LINEWIDTH / 4)</c> — integer again, so 35 becomes 8 and the 79
    /// of a 2.25 pt rule becomes 19.
    /// </description></item>
    /// <item><description>
    /// <c>impGetLineStyle</c> (<c>svx/source/table/viewcontactoftableobj.cxx</c>:183-184) hands
    /// that number to <c>svx::frame::Style</c> scaled by
    /// <c>o3tl::convert(1.0, twip, mm100)</c> — it reads the hundredths of a millimetre as
    /// <b>twips</b>. One unit is therefore drawn as 1/20 pt.
    /// </description></item>
    /// </list>
    /// <para>
    /// The whole chain is <c>max(1, floor(emu / 360) / 4) / 20</c> points, and it is measured at
    /// both ends: <c>Thailand17.ppt</c> page 11's group states 12700 and 28575 EMU, 26.2.4.2's own
    /// flat ODP of the deck states the cells' borders as <c>0.23pt</c> and <c>0.54pt</c> — 8 and
    /// 19 hundredths of a millimetre — and its PDF strokes them at <b>0.40 pt and 0.9499 pt</b>,
    /// which is 8/20 and 19/20 exactly. Reproduced rather than corrected, because the reference is
    /// what a comparison is against; the truncation at two of the three steps is why a 1 pt rule
    /// loses 60 % and a 2.25 pt rule 58 %.
    /// </para>
    /// <para>
    /// The <c>.pptx</c> path has its own version of the same ending —
    /// <c>DrawingTableGeometry.BorderWidth</c> — which rounds where this truncates and halves
    /// where this quarters, because <c>oox</c> and <c>msfilter</c> write the
    /// <c>BorderLine2</c> differently. They are deliberately not one function.
    /// </para>
    /// </remarks>
    internal static Length TableBorderWidth(uint emu)
    {
        long hundredthsOfMillimetre = emu / 360;
        long quarter = Math.Max(1, hundredthsOfMillimetre / 4);

        return Length.FromEmu(quarter * Length.EmuPerTwip);
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

    /// <summary>
    /// The rectangle a shape occupies once it has resized itself around its own text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong><c>fFitShapeToText</c> is the shape growing, not the text shrinking</strong>, and
    /// it is the one of the two that this reader had no answer for at all. Escher states it as
    /// bit 1 of <c>DFF_Prop_FitTextToShape</c> (191); <c>svdfppt.cxx</c>:1049-1110 turns it into
    /// <c>SdrTextAutoGrowHeightItem</c> on every horizontal text object it builds — the
    /// <c>SdrObjCustomShape</c> branch at <c>:1053-1055</c> and the <c>SdrRectObj</c> branch at
    /// <c>:1090</c> both — and into auto-grow-<em>width</em> instead when the text is vertical
    /// (<c>:1077-1080</c>), which is why a turned body is left alone here.
    /// </para>
    /// <para>
    /// The arithmetic is <c>SdrTextObj::AdjustTextFrameWidthAndHeight</c>
    /// (<c>svx/source/svdraw/svdotxat.cxx</c>:44-236) and its custom-shape twin
    /// (<c>svx/source/svdraw/svdoashp.cxx</c>:2249-2421), which agree line for line on the height:
    /// the text is broken to the frame's <em>open</em> width less the two horizontal distances and
    /// floored at two units, the height is the outliner's plus <b>one unit of tolerance</b>, that
    /// is clamped to the minimum frame height, and only then are the two vertical distances added.
    /// The growth then moves the edge the vertical anchor does not hold: the bottom for a
    /// top-anchored body, the top for a bottom-anchored one, and half of it each way for a
    /// centred one.
    /// </para>
    /// <para>
    /// <strong>The minimum is where the two branches differ, and it decides whether a shape may
    /// shrink.</strong> <c>makeSdrTextMinFrameHeightItem</c> is set only on the
    /// <c>SdrRectObj</c> branch (<c>svdfppt.cxx</c>:1116-1119), to the stated rectangle's height
    /// less the two vertical distances (<c>:982</c>) — so a real placeholder can only grow. A
    /// custom shape gets no such item, its minimum falls through to 1, and a box with less text
    /// than it has room for shrinks to fit. The branch is taken on exactly one condition
    /// (<c>:1041-1055</c>): the text object's kind is rewritten to Rectangle, and therefore the
    /// custom shape keeps its own text, unless the shape names a placeholder <em>and</em> its
    /// text kind is not <c>Other</c>.
    /// </para>
    /// <para>
    /// Measured on <c>pres_ioc_phuket.ppt</c> page 26, whose dark blue banner states 519 master
    /// units — 64.88 pt — and holds two empty 40 pt paragraphs: 26.2.4.2 resolves it to 3.647 cm,
    /// <b>103.38 pt</b>, and drew a bar 38.5 pt short of the reference's before this. Over the
    /// corpus's 51 <c>.ppt</c>, <b>1691 live slide shapes in 38 documents state the bit</b> and
    /// 1348 of those hold text; the reference's own flat-ODP export disagrees with the stated
    /// anchor by more than a millimetre on <b>101 shapes in 22 documents</b> — 83 taller and 18
    /// shorter (<c>probes/slides-r97/growth.py</c>).
    /// </para>
    /// <para>
    /// Not modelled: the rotation compensation at <c>svdotxat.cxx</c>:229-236, which moves the
    /// grown rectangle so that a rotated shape's own top-left stays where it was — a rotated
    /// shape grows about its centre here instead. <strong>That is unmeasured rather than measured
    /// as nil</strong>: the census above matches on <c>svg:x</c>/<c>svg:y</c>, which a rotated
    /// shape's export does not state at all, so it can say nothing about one.
    /// </para>
    /// </remarks>
    private DocRect Fitted(
        EscherShape shape, PptShapeText text, DocRect local, string? preset, double? adjustment)
    {
        // A turned body grows the shape's width instead, which this does not model.
        if (text.Flow != VerticalText.None) return local;

        if ((shape.Properties.Value(PptShapeGeometry.FitTextToShape, 0)
             & PptShapeGeometry.FitShapeToText) == 0)
        {
            return local;
        }

        DocRect frame = TextRectangle(shape, preset, local.Size, adjustment);
        if (frame.Height <= Length.Zero || local.Height <= Length.Zero) return local;

        Margins insets = Insets(shape);
        long across = insets.Left.Mm100 + insets.Right.Mm100;
        long down = insets.Top.Mm100 + insets.Bottom.Mm100;

        // `Size aSiz(rR.GetSize()); aSiz.AdjustWidth(-1);` is the open width, and the floor of two
        // is the reference's own (`svdotxat.cxx`:113-116).
        Length width = Length.FromMm100(Math.Max(frame.Width.Mm100 - across, 2));

        long height = SlideTextLayout.Height(text.Body, width, _fonts).Mm100 + 1;
        height = Math.Clamp(height, MinimumFrameHeight(shape, text, frame, down), MaximumFrameHeight);
        height += down;
        if (height < 1) height = 1;

        long growth = height - frame.Height.Mm100;
        if (growth == 0) return local;

        // `ImpCalculateTextFrame` (`svdoashp.cxx`:2424-2450) scales the text frame's movement back
        // onto the shape's own rectangle, which is the identity whenever the preset's text area is
        // the whole shape and a proportion whenever it is not.
        double scale = (double)local.Height.Mm100 / frame.Height.Mm100;
        long half = growth / 2;
        (long top, long bottom) = Anchor(shape) switch
        {
            TextAnchor.Bottom => (-growth, 0L),
            TextAnchor.Middle => (-half, growth - half),
            _ => (0L, growth),
        };

        Length newTop = local.Y + Length.FromMm100((long)(top * scale));
        Length newBottom = local.Bottom + Length.FromMm100((long)(bottom * scale));
        return new DocRect(local.X, newTop, local.Width, newBottom - newTop);
    }

    /// <summary>
    /// The ceiling a grown frame's text height is clamped to, in hundredths of a millimetre.
    /// </summary>
    /// <remarks>
    /// <c>Size aMaxSiz(100000, 100000)</c> — a metre — which the model's own
    /// <c>GetMaxObjSize()</c> may narrow and never widens (<c>svdotxat.cxx</c>:76-84,
    /// :98-108). No corpus shape reaches it; it is here because the reference has it and a
    /// runaway measurement would otherwise draw a shape off the deck.
    /// </remarks>
    private const long MaximumFrameHeight = 100000;

    /// <summary>
    /// The floor the reference puts under a grown frame's text height, in hundredths of a
    /// millimetre.
    /// </summary>
    /// <remarks>
    /// See <see cref="Fitted"/>: a shape whose text object was built as an <c>SdrRectObj</c> —
    /// a real placeholder — carries <c>makeSdrTextMinFrameHeightItem(rTextRect.GetHeight() -
    /// (nTextTop + nTextBottom))</c> and can therefore only grow; everything else falls through
    /// to the item default of zero, which the fit raises to one.
    /// </remarks>
    private long MinimumFrameHeight(
        EscherShape shape, PptShapeText text, DocRect frame, long down)
    {
        bool placeholder = text.Kind != PptTextKind.Other
                           && PlaceholderOf(shape) != PptPlaceholders.None;

        if (!placeholder) return 1;

        return Math.Max(frame.Height.Mm100 - down, 1);
    }

    /// <summary>A shape's text body, with the two things the fit and the layout both need.</summary>
    /// <param name="Body">The body, already turned if the shape's flow turns it.</param>
    /// <param name="Flow">Which way the shape's own text runs.</param>
    /// <param name="Kind">The text kind its <c>TextHeaderAtom</c> names.</param>
    private sealed record PptShapeText(SlideTextBody Body, VerticalText Flow, PptTextKind Kind);

    /// <summary>
    /// The text a shape holds, built once and used both by the fit and by the layout.
    /// </summary>
    /// <remarks>
    /// Built before the shape is placed because <see cref="Fitted"/> resizes the shape around it:
    /// the fill, the outline and the picture all have to be measured in the rectangle the text
    /// leaves behind, not in the one the anchor states.
    /// </remarks>
    private PptShapeText? TextIn(EscherShape shape, Context context)
    {
        if (TextOf(shape, context) is not { } run) return null;

        VerticalText flow = Flow(shape);
        if (PptTextBody.Build(run, context.Styles, context.Scheme, _fontTable,
                              Insets(shape), Anchor(shape), Wraps(shape),
                              Autofits(shape, run, context.InTable), _bulletPictures) is not { } body)
        {
            return null;
        }

        if (flow != VerticalText.None)
        {
            body = body with { Rotation = Quarter(flow) };
        }

        return new PptShapeText(body, flow, run.Kind);
    }

    private PlacedText? Text(
        PptShapeText? text,
        DocRect local,
        EscherShape shape,
        string? preset,
        double? adjustment,
        AffineTransform placement)
    {
        if (text is null) return null;

        SlideTextBody body = text.Body;
        VerticalText flow = text.Flow;

        DocRect rectangle = TextRectangle(shape, preset, local.Size, adjustment);

        // A body that turns its own text can never be laid out in the shape's own upright
        // rectangle: the lines run down the shape and break at its height, and the runs have to
        // travel with a matrix. This is the same construction as PptxSlideLayout.Turned and is
        // deliberately not a second one — the two formats spell the turn differently and mean the
        // same thing by it.
        if (flow != VerticalText.None)
        {
            return Turned(body, rectangle, placement, transpose: flow == VerticalText.Clockwise);
        }

        // Upright means "axis-aligned and not mirrored", not "the identity". A group scales its
        // children, so the factor is often neither one nor a rotation. Treating that as rotated
        // puts the shape's text into the shape's own coordinate space behind a matrix, where it is
        // correct on the page and unreadable to every tool that compares a pen position against a
        // reference renderer's. Mapping the rectangle instead is also what LibreOffice does: a
        // group scale resizes the shape and the text is laid out again inside it at its own size,
        // rather than being stretched.
        //
        // Both halves of that are now measured rather than read —
        // research/probes/slides-r23/group-scale-census.sh instruments this line and folds every
        // .ppt in the corpus. Of 7514 text shapes taking this branch, 7364 have an exactly unit
        // placement and 150 do not, across 11 of 51 documents, at factors from 0.00063 to 1.385.
        // The claim this comment used to make — that the commonest group's factor is 1.00025 —
        // is false: the whole band under 0.1% is empty, so the branch is load-bearing rather than
        // a rounding guard. And the reading it defends holds on hofman.ppt, whose 34 scaled
        // shapes carry A = 1.1163 and D = 1.0315: the reference draws 24.01, 28.01, 32.00 and
        // 43.99 pt there and so do we, where a scale reaching the font would give 26.8 to 49.1.
        bool upright = placement is { B: 0, C: 0, A: > 0, D: > 0 };
        DocRect area = upright
            ? new DocRect(
                ShapeTransform.Apply(placement, rectangle.Origin),
                new DocSize(
                    Length.FromEmu((long)Math.Round(rectangle.Width.Emu * placement.A)),
                    Length.FromEmu((long)Math.Round(rectangle.Height.Emu * placement.D))))
            : rectangle;

        List<PlacedPicture> markers = [];
        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, _fonts, markers);
        return runs.Count == 0 && markers.Count == 0
            ? null
            : new PlacedText(runs, upright ? AffineTransform.Identity : placement)
            {
                MarkerPictures = markers.Count == 0 ? null : markers,
            };
    }

    /// <summary>
    /// Lays a body out in a text frame turned inside its shape, and gives it the matrix.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The two arms are not the same construction and the format is the reason.</strong>
    /// A <em>vertical</em> flow (<c>TtoBA</c>, <c>TtoBN</c>, <c>VertN</c>) makes the text object
    /// vertical, so the lines run down the shape and break at its <em>height</em>: the rectangle
    /// transposes about its own centre, exactly as <c>PptxSlideLayout.Turned</c> does for
    /// <c>a:bodyPr/@vert</c>. <c>BtoT</c> does something else — <c>svdfppt.cxx:819-821</c> adds
    /// 90° to the text object's angle and leaves it horizontal, and the object is laid out in the
    /// shape's own rectangle and only then rotated (<c>svdfppt.cxx:1174-1188</c>,
    /// <c>NbcRotate</c> about the bound rect's centre). So its lines break at the shape's
    /// <em>width</em> and the rectangle does not transpose.
    /// </para>
    /// <para>
    /// The corpus's 51 <c>.ppt</c> carry <b>33 non-zero flows and every one of them is
    /// <c>TtoBA</c></b>, so the second arm has no corpus reach at all and is here because getting
    /// it wrong silently is worse than not having it: it is measured on the same probe.
    /// </para>
    /// </remarks>
    private PlacedText? Turned(
        SlideTextBody body, DocRect rectangle, AffineTransform placement, bool transpose)
    {
        double centreX = rectangle.X.Emu + (rectangle.Width.Emu / 2.0);
        double centreY = rectangle.Y.Emu + (rectangle.Height.Emu / 2.0);

        DocRect area = transpose
            ? new DocRect(
                Length.FromEmu((long)(centreX - (rectangle.Height.Emu / 2.0))),
                Length.FromEmu((long)(centreY - (rectangle.Width.Emu / 2.0))),
                rectangle.Height,
                rectangle.Width)
            : rectangle;

        List<PlacedPicture> markers = [];
        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, _fonts, markers);
        if (runs.Count == 0 && markers.Count == 0) return null;

        double halfWidth = rectangle.Width.Emu / 2.0;
        double halfHeight = rectangle.Height.Emu / 2.0;
        AffineTransform about = AffineTransform.Concat(
            AffineTransform.Concat(
                AffineTransform.Translation(-(rectangle.X.Emu + halfWidth), -(rectangle.Y.Emu + halfHeight)),
                AffineTransform.Rotation(body.Rotation)),
            AffineTransform.Translation(rectangle.X.Emu + halfWidth, rectangle.Y.Emu + halfHeight));

        return new PlacedText(runs, AffineTransform.Concat(about, placement))
        {
            MarkerPictures = markers.Count == 0 ? null : markers,
        };
    }

    /// <summary>Which quarter turn <c>txflTextFlow</c> asks for, if any.</summary>
    private enum VerticalText
    {
        /// <summary>The text reads across — <c>mso_txflHorzN</c> and <c>mso_txflHorzA</c>.</summary>
        None,

        /// <summary>The text reads downwards — <c>mso_txflTtoBA</c>, <c>TtoBN</c>, <c>VertN</c>.</summary>
        Clockwise,

        /// <summary>The text reads upwards — <c>mso_txflBtoT</c>.</summary>
        Anticlockwise,
    }

    /// <summary>
    /// Reads <c>txflTextFlow</c> as a quarter turn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SdrPowerPointImport::ImportShape</c> (<c>svdfppt.cxx:815-832</c>) splits the six values
    /// three ways: <c>mso_txflBtoT</c> adds 90° to the text object's own angle, the three
    /// top-to-bottom values set the object <em>vertical</em> instead, and the two horizontal ones
    /// do nothing. A vertical text object is the same rendering as DrawingML's
    /// <c>a:bodyPr/@vert</c> — the frame is transposed and the runs carry a quarter-turn matrix —
    /// so this produces the same <see cref="SlideTextBody.Rotation"/> that reader produces.
    /// </para>
    /// <para>
    /// <b>Measured on 26.2.4.2, six of six, and the fixture was built to discriminate.</b>
    /// <c>probes/slides-r56/patch-textflow.py</c> rewrites the four value bytes of this property
    /// in a corpus <c>.ppt</c> and leaves every other byte alone, so the six arms differ in
    /// nothing else — and, unlike a fixture exported by the reference, none of them states the
    /// reference's own default. On <c>concepts-surrounding-cloud-computing…ppt</c> page 11 the
    /// two vertical labels come out:
    /// </para>
    /// <code>
    ///   0 HorzN   upright        4 HorzA   upright
    ///   1 TtoBA   0 1 -1 0       3 TtoBN   0 1 -1 0       5 VertN  0 1 -1 0
    ///   2 BtoT    0 -1 1 0
    /// </code>
    /// <para>
    /// 24 text blocks apiece and identical pens across 1, 3 and 5. That refutes both rivals: "any
    /// non-zero value turns the same way" fails on 2, whose matrix is the other quarter and whose
    /// first pen is (43.14, 349.00) against (34.81, 309.86); and "only <c>TtoBA</c> is vertical"
    /// fails on 3 and 5. <c>0 1 -1 0</c> is the matrix the reference writes for
    /// <c>a:bodyPr/@vert</c>, which is what says the two spellings are one rendering.
    /// </para>
    /// </remarks>
    private static VerticalText Flow(EscherShape shape)
        => (shape.Properties.Value(EscherPropertyIds.TextFlow, 0) & 0xFFFF) switch
        {
            1 or 3 or 5 => VerticalText.Clockwise,
            2 => VerticalText.Anticlockwise,
            _ => VerticalText.None,
        };

    /// <summary>The turn a flow asks for, clockwise, in radians.</summary>
    private static double Quarter(VerticalText flow) => flow switch
    {
        VerticalText.Clockwise => Math.PI / 2.0,
        VerticalText.Anticlockwise => -Math.PI / 2.0,
        _ => 0,
    };

    /// <summary>
    /// The four text insets, which on this path are <b>not</b> rotated with the flow.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is where the two formats genuinely differ, and reusing the DrawingML answer
    /// here was wrong.</strong> <c>oox</c>'s <c>TextBodyProperties::pushTextDistances</c> shifts
    /// the four cyclically before they become <c>SdrText*DistItem</c>s; <c>svdfppt.cxx:857-880</c>
    /// hands <c>dxTextLeft</c>, <c>dyTextTop</c>, <c>dxTextRight</c> and <c>dyTextBottom</c>
    /// straight to <c>makeSdrTextLeftDistItem</c> and its three siblings with no shift at all. The
    /// dist items are then applied in the text object's <em>own</em> coordinates, which for a
    /// vertical object are already the transposed frame — so the identity here is the whole of it.
    /// </para>
    /// <para>
    /// Measured rather than argued, and by a fixture that isolates one slot at a time:
    /// <c>probes/slides-r56/make-ppt-inset-probe.py</c> writes five <c>.ppt</c> — all four insets
    /// zero, then 40 pt on exactly one edge — and differences each against the zero arm, which
    /// cancels the first line's ascent instead of having to know it. On 26.2.4.2, with the flow
    /// patched to <c>TtoBA</c> and then to <c>BtoT</c>:
    /// </para>
    /// <code>
    ///                  TtoBA  (0 -1 1 0)        BtoT  (0 1 -1 0)
    ///   lIns 40 pt     dx   0.00  dy -40.00     dx   0.00  dy +39.99
    ///   tIns 40 pt     dx -39.99  dy   0.00     dx +40.00  dy   0.00
    ///   rIns 40 pt     dx   0.00  dy   0.00     dx   0.00  dy   0.00
    ///   bIns 40 pt     dx   0.00  dy   0.00     dx   0.00  dy   0.00
    /// </code>
    /// <para>
    /// The left inset moves the block along the line and the top inset moves it across, in both
    /// directions — which is the identity, and not the cyclic shift
    /// <c>PptxTextBody.Insets</c> applies. A shifted reading puts every turned <c>.ppt</c> label
    /// 3.4 pt out on both axes on a shape that states nothing, because this format's defaults are
    /// 0.25 cm across and 0.13 cm down.
    /// </para>
    /// </remarks>
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
    /// Whether a shape's text is shrunk until it fits — the binary format's answer to
    /// <c>a:normAutofit</c>, which it does not spell anywhere.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Nothing in the file asks for it.</strong> The PowerPoint import turns autofit on
    /// from the <em>kind</em> of text a shape holds: a <c>TextHeaderAtom</c> naming Body, HalfBody
    /// or QuarterBody gets <c>TextFitToSizeType_AUTOFIT</c> and every other kind — a title, a
    /// subtitle, an ordinary shape's own text — gets none
    /// (<c>filter/source/msfilter/svdfppt.cxx:1030-1099</c>). So an outline placeholder shrinks to
    /// fit and the title above it does not, which is what a deck written in PowerPoint 97 looks
    /// like when it is opened today.
    /// </para>
    /// <para>
    /// The two exceptions are the shape growing instead of the text: <c>fFitShapeToText</c> makes
    /// the box taller, and a non-wrapping box makes it wider, and the import suppresses autofit
    /// for either (fdo#41245 — "autofit text only if there is no auto grow height and width").
    /// </para>
    /// <para>
    /// <strong>The wrap half of that is an approximation, and the corpus does not witness the
    /// difference — measured, after a round was briefed that it does.</strong> The reference
    /// derives auto-grow-<em>width</em> from the wrap only where the object it built is an
    /// <c>SdrObjCustomShape</c> <em>and</em> the text kind had been rewritten to Rectangle
    /// (<c>svdfppt.cxx</c>:1053-1055); every other branch sets <c>bAutoGrowWidth = false</c>
    /// (<c>:1084</c>) and the wrap decides nothing. The rewrite happens on exactly one condition —
    /// <c>!aTextObj.GetOEPlaceHolderAtom() || nPlaceholderId == PptPlaceholder::NONE</c>
    /// (<c>:1043-1047</c>) — so the two rules disagree only on a Body-kind text that <em>names
    /// itself a placeholder</em> and states <c>wrapNone</c>.
    /// </para>
    /// <para>
    /// <strong>The corpus holds no such shape.</strong> Of 1401 Body/HalfBody/QuarterBody shapes in
    /// the 51 <c>.ppt</c>, 55 state <c>wrapNone</c> — in <c>Architecture.ppt</c> and
    /// <c>Fundamentals_Module_1_basics.ppt</c> — and <strong>every one of the 55 carries no
    /// <c>OEPlaceholderAtom</c> at all</strong>, which is precisely the case where the reference
    /// takes the wrap. Corroborated at the reference: over every page of both decks, the set of
    /// drawn text sizes agrees with 26.2.4.2's exactly, the one exception being two classes on
    /// page 6 of <c>Fundamentals</c> that are an embedded chart the reference does not draw as text
    /// at all. <c>probes/ppt-fit-r85/placeholder-census.py</c>; round 84's
    /// <em>"55 shapes in 2 documents would newly autofit"</em> counted the wrap and not the
    /// placeholder atom, and is withdrawn.
    /// </para>
    /// <para>
    /// Measured on <c>berlin.ppt</c>, whose 29 slides are all outline placeholders: without this
    /// the text overflows the shape, runs off the bottom of the slide and is clipped away by the
    /// page, losing 39 of 1395 words with the page count still exactly right.
    /// </para>
    /// <para>
    /// <strong>And a table cell never autofits, whatever kind of text it holds</strong> —
    /// <paramref name="inTable"/>. A <c>.ppt</c> writes a table as a plain <em>group</em> of
    /// rectangles, each of which is an ordinary Body-kind text shape and each of which this rule
    /// would otherwise shrink on its own; the reference throws the group away and builds one
    /// <c>SdrTableObj</c> from it (<c>svdfppt.cxx</c>:2913, <c>CreateTable</c> at :7569), copying
    /// each rectangle's <c>OutlinerParaObject</c> into a cell and <em>nothing else of its item
    /// set</em>. <c>ApplyCellAttributes</c> (:7412) carries the four text distances, the two
    /// adjusts, the writing mode and the fill; <c>SDRATTR_TEXT_FITTOSIZE</c> is not among them,
    /// and <c>svx/source/table</c> mentions autofit nowhere at all. So the fit the group's members
    /// were given at :1099 is discarded on the way into the table.
    /// </para>
    /// <para>
    /// <strong>Confirmed twice.</strong> In 26.2.4.2's own flat ODP of
    /// <c>slides/ceiling-001/ppt/Thailand17.ppt</c> the deck's autofitted bodies export
    /// <c>style:shrink-to-fit="true"</c> and every one of page 11's 54
    /// <c>table:table-cell</c> styles states none, all 42 of its spans stating a flat
    /// <c>fo:font-size="12pt"</c>; in its own PDF of the same file every one of the page's 77
    /// body spans is drawn at 11.99 pt, where this tree drew the 35 spans whose cell wraps to two
    /// lines at 11.00 — <c>round(12 × 0.925)</c>, <c>constScaleLevels</c>' second row.
    /// </para>
    /// </remarks>
    /// <param name="shape">The shape carrying the text.</param>
    /// <param name="run">Its text, whose <c>TextHeaderAtom</c> kind decides the rule.</param>
    /// <param name="inTable">Whether the shape is a member of a group the reference tables.</param>
    internal static bool Autofits(EscherShape shape, PptTextRun run, bool inTable = false)
    {
        ArgumentNullException.ThrowIfNull(shape);
        ArgumentNullException.ThrowIfNull(run);

        if (inTable) return false;

        if (run.Kind is not (PptTextKind.Body or PptTextKind.HalfBody or PptTextKind.QuarterBody))
        {
            return false;
        }

        bool growsToText =
            (shape.Properties.Value(PptShapeGeometry.FitTextToShape, 0) & PptShapeGeometry.FitShapeToText) != 0;

        return !growsToText && Wraps(shape);
    }

    /// <summary>
    /// Whether a group shape is really a table.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two properties are in the group descriptor's <em>tertiary</em> table,
    /// <c>msofbtUDefProp</c>, and the reference reads them only from a shape holding no text of
    /// its own (<c>svdfppt.cxx</c>:1202-1240): <c>DFF_Prop_tableProperties</c> with either low bit
    /// set, and <c>DFF_Prop_tableRowProperties</c> present and non-empty. A group answering both
    /// is replaced wholesale by an <c>SdrTableObj</c> at <c>:2913</c>.
    /// </para>
    /// <para>
    /// This tree still draws the group as the rectangles it literally is — which is what the
    /// reference's table decomposes back into — and takes three consequences of the substitution:
    /// a cell's text is not autofitted, its row heights come from <see cref="PptTableRows"/>
    /// rather than from the members' own anchors, and a rule is drawn at
    /// <see cref="TableBorderWidth"/> rather than at the width the file states. See
    /// <c>probes/slides-bullet2-r112/results.md</c> and <c>probes/slide-table-r113/results.md</c>.
    /// </para>
    /// <para>
    /// Censused over the 51 <c>.ppt</c> of the corpus
    /// (<c>probes/slides-bullet2-r112/tablecensus.py</c>): <strong>34 table groups in 10
    /// documents, 1089 members, of which 391 in 5 documents hold Body-kind text</strong> and are
    /// therefore the reach of the arm above.
    /// </para>
    /// </remarks>
    internal static bool IsTableGroup(EscherShape group)
        => (group.TertiaryProperties.Value(PptShapeGeometry.TableProperties, 0) & 3) != 0
           && !group.TertiaryProperties.Data(PptShapeGeometry.TableRowProperties).IsEmpty;

    /// <summary>
    /// The rows a table group's members are re-laid onto, or null when nothing moves.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>CreateTable</c> (<c>svdfppt.cxx</c>:7569) seeds the table's rows from the non-line
    /// members' snap-rect tops and measures the last one to the group's own bottom
    /// (<c>CreateTableRows</c>, :7339), and <c>TableLayouter::LayoutTableHeight</c>
    /// (<c>svx/source/table/tablelayouter.cxx</c>:724) then raises each to
    /// <c>max(stated, minimum)</c> before <see cref="PptTableRows"/>'s own remark takes over. The
    /// minimum is the tallest single-row cell's <c>Cell::getMinimumHeight</c>
    /// (<c>svx/source/table/cell.cxx</c>:686): its text laid out at the cell's width less the two
    /// horizontal distances, plus one hundredth of a millimetre, plus the two vertical distances.
    /// A cell spanning rows is deferred to its last row.
    /// </para>
    /// <para>
    /// <strong>A row none of whose cells can be measured keeps what the file states.</strong> The
    /// reference measures an empty cell as one empty paragraph of the draw outliner's own font,
    /// which is not in the file and cannot be recovered from it; a zero would be worse than the
    /// stated height in both directions, because the second layout can shrink a row as well as
    /// grow it and would collapse such a row outright. Measured: it is the last row of
    /// <c>joint_user_outcomes_michael_fullerton_29.06.12.ppt</c> page 15, where the guard leaves
    /// 19.25 pt against the reference's 19.42 and dropping it leaves nothing at all.
    /// </para>
    /// <para>
    /// The arithmetic is done in the group's own child space, which is what a member's anchor is
    /// stated in; a minimum is measured on the page and divided back by the group's own scale,
    /// because the text is laid out at the size it is drawn at.
    /// </para>
    /// </remarks>
    private PptTableRows? TableRows(EscherShape group, Context context, AffineTransform inner)
    {
        if (inner.A <= 0 || inner.D <= 0) return null;

        Context cells = context with { InTable = true };
        List<(EscherShape Shape, long Top, long Bottom, long Width)> members = [];
        long groupTop = long.MaxValue;
        long groupBottom = long.MinValue;
        SortedSet<long> tops = [];

        foreach (EscherShape child in group.Children)
        {
            if (AnchorOf(child) is not { } a) continue;

            long top = Units(a.Top).Emu;
            long bottom = top + Units(a.Height).Emu;
            groupTop = Math.Min(groupTop, top);
            groupBottom = Math.Max(groupBottom, bottom);

            if (IsTableRule(child)) continue;

            // A member narrower or shorter than one unit is not a cell (`GetCellPosition`,
            // `svdfppt.cxx`:7193) but it does seed a row, exactly as it does there.
            tops.Add(top);
            if (a.Width > 1 && a.Height > 1)
            {
                members.Add((child, top, bottom, Units(a.Width).Emu));
            }
        }

        if (tops.Count == 0 || groupBottom <= tops.Min) return null;

        long[] edges = [.. tops, groupBottom];
        long[] minimums = new long[edges.Length - 1];
        bool[] measured = new bool[minimums.Length];
        List<(int Last, int First, long Minimum)> spanning = [];

        foreach ((EscherShape shape, long top, long bottom, long width) in members)
        {
            int first = Array.BinarySearch(edges, top);
            if (first < 0 || first >= minimums.Length) continue;

            int last = first;
            while (last + 1 < minimums.Length && edges[last + 1] < bottom) last++;

            if (Minimum(shape, width) is not { } minimum) continue;

            if (last > first)
            {
                spanning.Add((last, first, minimum));
            }
            else
            {
                minimums[first] = Math.Max(minimums[first], minimum);
                measured[first] = true;
            }
        }

        // A row none of whose cells could be measured keeps what the file states, and is
        // therefore neither grown nor shrunk. See the remark above: the reference measures an
        // empty cell as one empty paragraph of the draw outliner's own font, which is not in the
        // file, and a zero here would let `distribute` collapse the row outright.
        for (int row = 0; row < minimums.Length; row++)
        {
            if (!measured[row]) minimums[row] = edges[row + 1] - edges[row];
        }

        // A row-spanning cell only ever grows its last row, and the rows it covers are subtracted
        // from what it needs -- except when the span starts at row zero, where the reference's own
        // loop guard skips the subtraction outright (`tablelayouter.cxx`:830).
        foreach ((int last, int first, long minimum) in spanning)
        {
            long remaining = minimum;
            for (int row = Math.Max(first, 1); row < last; row++)
            {
                remaining -= Math.Max(edges[row + 1] - edges[row], minimums[row]);
            }

            minimums[last] = Math.Max(minimums[last], remaining);
        }

        return PptTableRows.Of(edges, groupTop, groupBottom, minimums);

        long? Minimum(EscherShape shape, long width)
        {
            if (TextIn(shape, cells) is not { } text) return null;
            if (text.Flow != VerticalText.None) return null;

            Margins insets = Insets(shape);
            Length textWidth = Length.FromEmu(
                (long)(width * inner.A) - insets.Left.Emu - insets.Right.Emu);

            if (textWidth <= Length.Zero) return null;

            Length height = SlideTextLayout.Height(text.Body, textWidth, _fonts);
            long onPage = height.Emu + Length.EmuPerMm100 + insets.Top.Emu + insets.Bottom.Emu;

            return (long)(onPage / inner.D);
        }
    }

    /// <summary>
    /// A shape's text, whether it holds the characters itself or refers to the slide list.
    /// </summary>
    private PptTextRun? TextOf(EscherShape shape, Context context)
    {
        if (shape.ClientTextbox is not { } textbox) return null;

        int start = textbox.ContentStart;
        int end = _stream.EndOf(textbox);

        // Picture bullets and automatic numbering are not in the text box at all -- they are in
        // the shape's own private data, three records down and behind an atom that holds records.
        // See `PptTextReader.ReadExtendedParagraphs`.
        IReadOnlyList<PptExtendedParagraph>? extended =
            PptTextReader.ReadExtendedParagraphs(_stream, shape.ClientData);

        foreach (DffRecordHeader record in _stream.Range(start, end))
        {
            if (record.Type != PptRecordTypes.OutlineTextRefAtom) continue;

            uint reference = DffRecordBuffer.ReadUInt32(_stream.Content(record));

            // A shape that refers to the slide list still carries its own extensions and its own
            // ruler, and both still apply to the text it points at. See `PptTextReader.RulerIn`:
            // the reference remembers the ruler's offset before it patches the client textbox
            // header over to the referenced text, and every ruler in the two decks this decides
            // is in a textbox that refers out.
            PptTextRuler? ruler = PptTextReader.RulerIn(_stream, start, end);

            return OutlineText(context.Entry, reference, context.Fields) is { } outline
                ? outline with
                  {
                      Extended = extended ?? outline.Extended,
                      Ruler = ruler ?? outline.Ruler,
                  }
                : null;
        }

        return PptTextReader.Read(_stream, start, end, context.Fields, extended, _hyperlinks);
    }

    /// <summary>
    /// The <paramref name="reference"/>th text run of a slide's entry in the document's slide
    /// list.
    /// </summary>
    private PptTextRun? OutlineText(PptPageEntry entry, uint reference, PptFieldValues fields)
    {
        int matches = 0;
        int start = -1;

        foreach (DffRecordHeader record in _stream.Range(entry.TextStart, entry.TextEnd))
        {
            if (record.Type == PptRecordTypes.SlidePersistAtom) break;
            if (record.Type != PptRecordTypes.TextHeaderAtom) continue;

            if (start >= 0)
            {
                return PptTextReader.Read(
                    _stream, start, record.Position, fields, null, _hyperlinks);
            }

            if (matches++ == reference) start = record.Position;
        }

        return start >= 0
            ? PptTextReader.Read(_stream, start, entry.TextEnd, fields, null, _hyperlinks)
            : null;
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
    private static Dictionary<string, double>? Guides(double? adjustment)
        => adjustment is not { } value
            ? null
            : new Dictionary<string, double>(StringComparer.Ordinal)
            {
                ["adj"] = value,
                ["adj1"] = value,
            };

    /// <summary>
    /// The stated <c>adjustValue</c> in the units this shape's preset expects, at the size the
    /// shape is finally drawn at.
    /// </summary>
    /// <remarks>
    /// Taken at the size rather than once per shape because the conversion is not always a pure
    /// scale — see <see cref="PptShapeGeometry.Adjustment"/> — and a shape that grows around its
    /// own text changes its aspect ratio while it does so.
    /// </remarks>
    private static double? Adjustment(ushort shapeType, int? stated, DocSize size)
        => stated is { } value ? PptShapeGeometry.Adjustment(shapeType, value, size) : null;

    /// <summary>
    /// The rectangle the shape's text is laid out in, turned over for the presets whose two
    /// definitions are reflections of each other.
    /// </summary>
    private static DocRect TextRectangle(
        EscherShape shape, string? preset, DocSize size, double? adjustment)
    {
        DocRect rectangle = SlidePresetGeometry.TextRectangle(preset, size, Guides(adjustment));

        return PptShapeGeometry.MirrorsVertically(shape.ShapeType)
            ? rectangle with { Y = size.Height - rectangle.Bottom }
            : rectangle;
    }

    /// <summary>
    /// The expanded preset, turned over when the binary vocabulary's definition of this shape type
    /// is a vertical reflection of the DrawingML preset it is mapped to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reflection is applied to the geometry alone, in the shape's own coordinates, and
    /// <em>before</em> the placement matrix — which is where <c>fFlipV</c> lives. That order is the
    /// whole point: the flag is read correctly and always was, and applying it to a base that is
    /// already upside down inverts the shape whether the flag is set or not.
    /// </para>
    /// <para>
    /// The outline, its subpaths and the text rectangle turn over together, because the
    /// <c>a:rect</c> a preset states is attached to its own geometry — <c>trapezoid</c>'s is
    /// <c>r il it ir b</c>, which reaches the wide edge — and leaving it behind would put the text
    /// against the edge the shape no longer has there. The shape's picture, fill and placement are
    /// untouched: this mirror is a property of the <em>preset table</em>, not of the shape, and the
    /// shape's own <c>fFlipV</c> is applied afterwards by <see cref="Placement"/>.
    /// </para>
    /// </remarks>
    private static CustomShapeGeometry.Geometry Mirrored(
        CustomShapeGeometry.Geometry geometry, EscherShape shape, DocSize size)
    {
        if (!PptShapeGeometry.MirrorsVertically(shape.ShapeType)) return geometry;

        AffineTransform flip = new(1, 0, 0, -1, 0, size.Height.Emu);

        IReadOnlyList<PresetSubpath>? subpaths = geometry.Subpaths is { Count: > 0 } parts
            ? [.. parts.Select(part => part with { Outline = ShapeTransform.Apply(flip, part.Outline) })]
            : geometry.Subpaths;

        return new CustomShapeGeometry.Geometry(
            ShapeTransform.Apply(flip, geometry.Outline),
            geometry.TextRectangle with { Y = size.Height - geometry.TextRectangle.Bottom },
            subpaths);
    }
}
