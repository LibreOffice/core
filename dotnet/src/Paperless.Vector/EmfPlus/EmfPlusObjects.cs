using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Vector.Metafiles;

namespace Paperless.Vector.EmfPlus;

/// <summary>
/// One entry in the EMF+ object table.
/// </summary>
/// <remarks>
/// <b>EMF+ has its own object table and it is not GDI's.</b> Two hundred and fifty-six slots
/// named by the low byte of a record's flags word, with no create and no delete: an
/// <c>EmfPlusObject</c> record simply overwrites whatever was in the slot, and a slot that was
/// never written is a record that draws nothing. That is a different lifetime from
/// <c>GraphicsObjectTable</c>'s — GDI's lowest-free-slot rule, delete semantics and stock objects
/// have no EMF+ equivalent — which is why this is a separate table rather than a reuse.
/// </remarks>
internal abstract class EmfPlusObject;

/// <summary>
/// An EMF+ brush: solid, hatch, texture, path gradient or linear gradient.
/// </summary>
/// <remarks>
/// Ported from <c>drawinglayer/source/tools/emfpbrush.cxx</c>. A pen carries one at its end, which
/// is why <see cref="EmfPlusPen"/> derives from this rather than holding one.
/// </remarks>
internal class EmfPlusBrush : EmfPlusObject
{
    /// <summary>Which of the five kinds this is.</summary>
    public EmfPlusBrushType Type { get; private set; }

    /// <summary>The <c>BrushData</c> flags, which say which optional fields the record carried.</summary>
    public uint Data { get; private set; }

    /// <summary>How the gradient or texture repeats outside its own extent.</summary>
    public int WrapMode { get; private set; }

    /// <summary>The solid colour, a hatch's foreground, or a gradient's first colour.</summary>
    public Colour Colour { get; private set; } = Core.Graphics.Colour.Black;

    /// <summary>A hatch's background, or a gradient's second colour.</summary>
    public Colour SecondColour { get; private set; } = Core.Graphics.Colour.White;

    /// <summary>Which of the fifty-three hatch styles.</summary>
    public int HatchStyle { get; private set; }

    /// <summary>A linear gradient's start point, or a path gradient's centre.</summary>
    public (double X, double Y) FirstPoint { get; private set; }

    /// <summary>A linear gradient's extent.</summary>
    public (double Width, double Height) Extent { get; private set; }

    /// <summary>The brush's own transform, when it stated one.</summary>
    public AffineTransform? Transform { get; private set; }

    /// <summary>A blend curve's positions, paired with <see cref="BlendFactors"/>.</summary>
    public double[]? BlendPositions { get; private set; }

    /// <summary>A blend curve's factors: how far towards the second colour each position is.</summary>
    public double[]? BlendFactors { get; private set; }

    /// <summary>An explicit colour ramp's positions.</summary>
    public double[]? PresetPositions { get; private set; }

    /// <summary>An explicit colour ramp's colours.</summary>
    public Colour[]? PresetColours { get; private set; }

    /// <summary>A path gradient's colours at its boundary vertices.</summary>
    public Colour[]? SurroundColours { get; private set; }

    /// <summary>A path gradient's boundary.</summary>
    public EmfPlusPath? Boundary { get; private set; }

    /// <summary>A texture brush's image.</summary>
    public EmfPlusImage? Texture { get; private set; }

    /// <summary>Reads a brush from a record's cursor.</summary>
    /// <param name="stream">The cursor, positioned at the brush's version field.</param>
    public void Read(EmfPlusStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        stream.Skip(4);                 // version
        Type = (EmfPlusBrushType)stream.U32();

        switch (Type)
        {
            case EmfPlusBrushType.SolidColour:
                Colour = Argb(stream.U32());
                break;

            case EmfPlusBrushType.Hatch:
                HatchStyle = stream.I32();
                Colour = Argb(stream.U32());
                SecondColour = Argb(stream.U32());
                break;

            case EmfPlusBrushType.Texture:
                Data = stream.U32();
                WrapMode = stream.I32();
                if ((Data & EmfPlusBrushData.Transform) != 0) Transform = ReadTransform(stream);
                Texture = new EmfPlusImage();
                Texture.Read(stream);
                break;

            case EmfPlusBrushType.PathGradient:
                ReadPathGradient(stream);
                break;

            case EmfPlusBrushType.LinearGradient:
                ReadLinearGradient(stream);
                break;

            default:
                break;
        }
    }

    /// <summary>Reads a 32-bit ARGB colour, which EMF+ states everywhere GDI states a COLORREF.</summary>
    /// <remarks>
    /// <b>The alpha byte is real and is used.</b> GDI has no alpha at all, so a decoder carried
    /// over from the EMF side that masks the top byte away draws every partly transparent
    /// highlight in a chart as an opaque block.
    /// </remarks>
    public static Colour Argb(uint value) => Colour.FromArgb(value);

    /// <summary>Reads an EMF+ transform: six floats, row-major with the translation last.</summary>
    public static AffineTransform ReadTransform(EmfPlusStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        float m11 = stream.F32();
        float m12 = stream.F32();
        float m21 = stream.F32();
        float m22 = stream.F32();
        float dx = stream.F32();
        float dy = stream.F32();

        return new AffineTransform(m11, m12, m21, m22, dx, dy);
    }

    private void ReadPathGradient(EmfPlusStream stream)
    {
        Data = stream.U32();
        WrapMode = stream.I32();
        Colour = Argb(stream.U32());
        FirstPoint = (stream.F32(), stream.F32());

        int surround = (int)Math.Min(stream.U32(), 0x10000u);
        if (surround > stream.Remaining / 4) surround = stream.Remaining / 4;

        if (surround > 0)
        {
            SurroundColours = new Colour[surround];
            for (int i = 0; i < surround; i++) SurroundColours[i] = Argb(stream.U32());
            SecondColour = SurroundColours[0];
        }

        if ((Data & EmfPlusBrushData.Path) != 0)
        {
            int length = stream.I32();
            int at = stream.Offset;

            stream.Skip(4);             // the boundary path's own version
            int points = stream.I32();
            uint flags = stream.U32();
            Boundary = EmfPlusPath.Read(stream, points, flags, withTypes: true);

            if (length > 0) stream.SeekTo((long)at + length);
        }
        else
        {
            // A boundary stated as bare points carries no point-type array at all, so reading one
            // would consume the bytes that follow it — the brush transform, and then everything
            // else. LibreOffice reads it the same way and then seeks past 8 bytes a point.
            int points = stream.I32();
            int at = stream.Offset;
            Boundary = EmfPlusPath.Read(stream, points, 0, withTypes: false);
            if (points > 0) stream.SeekTo((long)at + ((long)points * 8));
        }

        if ((Data & EmfPlusBrushData.Transform) != 0) Transform = ReadTransform(stream);

        ReadBlend(stream);
    }

    private void ReadLinearGradient(EmfPlusStream stream)
    {
        Data = stream.U32();
        WrapMode = stream.I32();
        FirstPoint = (stream.F32(), stream.F32());
        Extent = (stream.F32(), stream.F32());
        Colour = Argb(stream.U32());
        SecondColour = Argb(stream.U32());

        // Two more colour fields follow, repeating the first two. [MS-EMFPLUS] 2.2.2.24 calls
        // them reserved; every producer writes the same pair again.
        stream.Skip(8);

        if ((Data & EmfPlusBrushData.Transform) != 0) Transform = ReadTransform(stream);

        ReadBlend(stream);
    }

    private void ReadBlend(EmfPlusStream stream)
    {
        // The two are mutually exclusive by [MS-EMFPLUS] 2.2.2.24; a brush that sets both is
        // malformed and neither is trusted.
        if ((Data & EmfPlusBrushData.PresetColours) != 0 && (Data & EmfPlusBrushData.BlendFactorsHorizontal) != 0)
        {
            return;
        }

        if ((Data & EmfPlusBrushData.BlendFactorsHorizontal) != 0)
        {
            int count = (int)Math.Min(stream.U32(), 0x10000u);
            if (count <= 0 || count > stream.Remaining / 8) return;

            BlendPositions = new double[count];
            BlendFactors = new double[count];

            for (int i = 0; i < count; i++) BlendPositions[i] = stream.F32();
            for (int i = 0; i < count; i++) BlendFactors[i] = stream.F32();
            return;
        }

        if ((Data & EmfPlusBrushData.PresetColours) == 0) return;

        int stops = (int)Math.Min(stream.U32(), 0x10000u);
        if (stops <= 0 || stops > stream.Remaining / 8) return;

        PresetPositions = new double[stops];
        PresetColours = new Colour[stops];

        for (int i = 0; i < stops; i++) PresetPositions[i] = stream.F32();
        for (int i = 0; i < stops; i++) PresetColours[i] = Argb(stream.U32());
    }
}

/// <summary>
/// An EMF+ pen: a width, a unit, a set of optional fields, and a brush.
/// </summary>
/// <remarks>
/// <b>The optional fields are positional.</b> Thirteen flag bits, each naming a field that is
/// present only when its bit is set, read in the order the bits are numbered — and the brush that
/// every pen ends with sits after all of them. Skipping a set bit, or reading an unset one, moves
/// the brush and turns a black hairline into whatever the dash array's first float looks like as a
/// colour. Ported from <c>drawinglayer/source/tools/emfppen.cxx</c>.
/// </remarks>
internal sealed class EmfPlusPen : EmfPlusBrush
{
    /// <summary>Which optional fields the record carried.</summary>
    public uint PenData { get; private set; }

    /// <summary>The unit <see cref="Width"/> is stated in.</summary>
    public EmfPlusUnit Unit { get; private set; }

    /// <summary>The pen's width, in <see cref="Unit"/>.</summary>
    public double Width { get; private set; }

    /// <summary>The cap at the start of a stroke.</summary>
    public int StartCap { get; private set; }

    /// <summary>The cap at the end of a stroke.</summary>
    public int EndCap { get; private set; }

    /// <summary>How corners are drawn.</summary>
    public LineJoin Join { get; private set; } = LineJoin.Miter;

    /// <summary>The miter limit.</summary>
    public double MiterLimit { get; private set; } = 10.0;

    /// <summary>One of the five predefined dash styles, or 5 for a custom array.</summary>
    public int DashStyle { get; private set; }

    /// <summary>How far into the dash pattern the stroke starts, as a multiple of the width.</summary>
    public double DashOffset { get; private set; }

    /// <summary>A custom dash array, as multiples of the pen width.</summary>
    public double[]? DashPattern { get; private set; }

    /// <summary>The outline of a custom start cap, in the cap's own space.</summary>
    public EmfPlusPath? CustomStartCap { get; private set; }

    /// <summary>The outline of a custom end cap, in the cap's own space.</summary>
    public EmfPlusPath? CustomEndCap { get; private set; }

    /// <summary>What a custom cap scales its own width by, on top of the pen's width.</summary>
    public double CustomCapScale { get; private set; } = 1.0;

    /// <summary>Reads a pen, then the brush it ends with.</summary>
    /// <param name="stream">The cursor, positioned at the pen's version field.</param>
    public void ReadPen(EmfPlusStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        stream.Skip(4);                 // version
        stream.Skip(4);                 // pen type, which is always 0
        PenData = stream.U32();
        Unit = (EmfPlusUnit)stream.U32();
        Width = stream.F32();

        // A width of zero means "the thinnest line the device draws", which has no
        // device-independent value; these are LibreOffice's two (emfppen.cxx).
        if (Width == 0.0) Width = Unit == EmfPlusUnit.World ? 0.18 : 0.05;

        if ((PenData & EmfPlusPenData.Transform) != 0) ReadTransform(stream);
        if ((PenData & EmfPlusPenData.StartCap) != 0) StartCap = stream.I32();
        if ((PenData & EmfPlusPenData.EndCap) != 0) EndCap = stream.I32();

        int join = 0;
        if ((PenData & EmfPlusPenData.Join) != 0)
        {
            join = stream.I32();
            Join = join switch
            {
                1 => LineJoin.Bevel,
                2 => LineJoin.Round,
                _ => LineJoin.Miter,
            };
        }

        if ((PenData & EmfPlusPenData.MiterLimit) != 0)
        {
            double limit = stream.F32();

            // EMF+ states the limit as a ratio, as PDF and Skia do, so it goes through as it
            // stands. LibreOffice converts it to a minimum angle because its stroke attribute is
            // stated that way; the ratio is the more direct of the two here.
            if (limit >= 1.0 && limit < 1e4) MiterLimit = limit;
        }

        if ((PenData & EmfPlusPenData.LineStyle) != 0) DashStyle = stream.I32();
        if ((PenData & EmfPlusPenData.DashedLineCap) != 0) stream.Skip(4);
        if ((PenData & EmfPlusPenData.DashedLineOffset) != 0) DashOffset = stream.F32();

        if ((PenData & EmfPlusPenData.DashedLine) != 0)
        {
            DashStyle = 5;
            int count = (int)Math.Min(stream.U32(), 0x1000u);

            if (count > 0 && count <= stream.Remaining / 4)
            {
                DashPattern = new double[count];
                for (int i = 0; i < count; i++) DashPattern[i] = stream.F32();
            }
        }

        if ((PenData & EmfPlusPenData.Alignment) != 0) stream.Skip(4);

        if ((PenData & EmfPlusPenData.CompoundLine) != 0)
        {
            int count = (int)Math.Min(stream.U32(), 0x1000u);
            stream.Skip(Math.Min(count * 4, stream.Remaining));
        }

        if ((PenData & EmfPlusPenData.CustomStartCap) != 0) CustomStartCap = ReadCustomCap(stream);
        if ((PenData & EmfPlusPenData.CustomEndCap) != 0) CustomEndCap = ReadCustomCap(stream);

        Read(stream);
    }

    /// <summary>
    /// Reads an <c>EmfPlusCustomLineCap</c> and answers the outline it decorates a line end with.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Ported from <c>EMFPCustomLineCap::Read</c>
    /// (<c>drawinglayer/source/tools/emfpcustomlinecap.cxx</c>). Only the <em>default</em> form
    /// carries a path; the adjustable-arrow form states a width, a height and a middle inset
    /// instead, which LibreOffice also reads and does not use.
    /// </para>
    /// <para>
    /// <b>The record's own length field is what makes this safe to get wrong.</b> A custom cap is
    /// the last thing before the pen's brush, so a misread of its interior would move the brush
    /// and the line would come out whatever the next float looks like as a colour. Seeking to the
    /// stated end rather than trusting the parse keeps that contained to the cap.
    /// </para>
    /// </remarks>
    private EmfPlusPath? ReadCustomCap(EmfPlusStream stream)
    {
        int length = stream.I32();
        int at = stream.Offset;
        EmfPlusPath? outline = null;

        stream.Skip(4);                 // the cap's own version
        uint type = stream.U32();

        if (type == 0)
        {
            uint flags = stream.U32();
            stream.Skip(4);             // base cap
            stream.Skip(4);             // base inset
            stream.Skip(12);            // the cap's own stroke start, end and join
            stream.Skip(4);             // miter limit
            CustomCapScale = stream.F32();
            stream.Skip(16);            // the fill and stroke hot spots

            // The fill path when there is one, else the line path: both describe the same
            // decoration and only one is drawn.
            if ((flags & 0x01) != 0) outline = ReadCapPath(stream);
            if ((flags & 0x02) != 0) outline ??= ReadCapPath(stream);
        }

        if (length > 0) stream.SeekTo((long)at + length);
        return outline;
    }

    /// <summary>Reads one of a custom cap's two paths.</summary>
    private static EmfPlusPath? ReadCapPath(EmfPlusStream stream)
    {
        stream.Skip(4);                 // the path's length, which the caller's bound covers
        stream.Skip(4);                 // the path's own version
        int points = stream.I32();
        uint flags = stream.U32();

        return EmfPlusPath.Read(stream, points, flags, withTypes: true);
    }

    /// <summary>The dash array a style word asks for, as multiples of the pen width.</summary>
    /// <remarks>
    /// GDI+ names five styles and states no lengths, exactly as GDI does. These ratios are
    /// LibreOffice's (<c>EMFPPen::GetStrokeAttribute</c>), and reproducing them rather than
    /// choosing our own matters because a dashed border with a different duty cycle reads as a
    /// different border.
    /// </remarks>
    public double[]? Dashes() => DashPattern ?? DashStyle switch
    {
        1 => [3, 1],
        2 => [1, 1],
        3 => [3, 1, 1, 1],
        4 => [3, 1, 1, 1, 1, 1],
        _ => null,
    };

    /// <summary>The cap a start or end cap code names, insofar as a stroke can state it.</summary>
    /// <remarks>
    /// Only the three plain caps map onto <see cref="LineCap"/>. The six anchor caps are arrow
    /// heads and diamonds — line <em>decorations</em>, which the drawing model has no place for —
    /// and they fall back to the flat cap rather than being approximated by a round one.
    /// </remarks>
    public static LineCap Cap(int code) => code switch
    {
        1 => LineCap.Square,
        2 => LineCap.Round,
        _ => LineCap.Butt,
    };

    /// <summary>
    /// True when the pen asks for a cap that is a <em>decoration</em> rather than a way of
    /// finishing the stroke, so a shape has to be drawn at the line's ends as well.
    /// </summary>
    /// <remarks>
    /// Seven of GDI+'s ten caps are decorations: the triangle, the four anchors and a custom
    /// path. <c>0x10</c> — <c>NoAnchor</c> — is in the anchor range and draws nothing, so it is
    /// excluded here rather than producing an empty decoration at every line end.
    /// </remarks>
    public bool HasCustomCap
        => CustomStartCap is not null
            || CustomEndCap is not null
            || Decorates(StartCap)
            || Decorates(EndCap);

    private static bool Decorates(int code) => code is 3 or 0x11 or 0x12 or 0x13 or 0x14;

    /// <summary>
    /// True when the pen names a custom cap that carried no outline, so nothing can be drawn
    /// for it.
    /// </summary>
    /// <remarks>
    /// The adjustable-arrow form of an <c>EmfPlusCustomLineCap</c> states a width, a height and
    /// a middle inset instead of a path. LibreOffice reads the same fields and does not use them
    /// either.
    /// </remarks>
    public bool HasUndrawnCap
        => ((PenData & EmfPlusPenData.CustomStartCap) != 0 && CustomStartCap is null)
            || ((PenData & EmfPlusPenData.CustomEndCap) != 0 && CustomEndCap is null);
}

/// <summary>An EMF+ font: a size, a unit, four style bits and a family name.</summary>
internal sealed class EmfPlusFont : EmfPlusObject
{
    /// <summary>The em size, in <see cref="Unit"/>.</summary>
    public double Size { get; private set; }

    /// <summary>The unit <see cref="Size"/> is stated in.</summary>
    public EmfPlusUnit Unit { get; private set; }

    /// <summary>The style bits: bold, italic, underline, strikeout.</summary>
    public int Style { get; private set; }

    /// <summary>The family name.</summary>
    public string Family { get; private set; } = string.Empty;

    /// <summary>True for bold.</summary>
    public bool IsBold => (Style & 0x01) != 0;

    /// <summary>True for italic.</summary>
    public bool IsItalic => (Style & 0x02) != 0;

    /// <summary>True for underlined.</summary>
    public bool IsUnderlined => (Style & 0x04) != 0;

    /// <summary>True for struck out.</summary>
    public bool IsStruckOut => (Style & 0x08) != 0;

    /// <summary>Reads a font object.</summary>
    /// <param name="stream">The cursor, positioned at the font's version field.</param>
    public void Read(EmfPlusStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        stream.Skip(4);                 // version
        Size = stream.F32();
        Unit = (EmfPlusUnit)stream.U32();
        Style = stream.I32();
        stream.Skip(4);                 // reserved
        int length = stream.I32();

        if (length is > 0 and < 0x4000) Family = stream.Utf16(length);
    }
}

/// <summary>An EMF+ string format: alignment, margins, tracking and direction.</summary>
internal sealed class EmfPlusStringFormat : EmfPlusObject
{
    /// <summary>The format flags, which carry direction and wrapping.</summary>
    public uint Flags { get; private set; }

    /// <summary>Horizontal alignment within the layout rectangle: near, centre or far.</summary>
    public int Alignment { get; private set; }

    /// <summary>Vertical alignment within the layout rectangle.</summary>
    public int LineAlignment { get; private set; }

    /// <summary>The left margin, as a multiple of the em size.</summary>
    public double LeadingMargin { get; private set; }

    /// <summary>The right margin, as a multiple of the em size.</summary>
    public double TrailingMargin { get; private set; }

    /// <summary>A multiplier on every character's advance; 1 leaves it alone.</summary>
    public double Tracking { get; private set; } = 1.0;

    /// <summary>True when the text runs right to left.</summary>
    public bool IsRightToLeft => (Flags & 0x00000001) != 0;

    /// <summary>True when the text runs top to bottom.</summary>
    public bool IsVertical => (Flags & 0x00000002) != 0;

    /// <summary>Reads a string format object.</summary>
    /// <param name="stream">The cursor, positioned at the format's version field.</param>
    public void Read(EmfPlusStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        stream.Skip(4);                 // version
        Flags = stream.U32();
        stream.Skip(4);                 // language
        Alignment = stream.I32();
        LineAlignment = stream.I32();
        stream.Skip(4);                 // digit substitution method
        stream.Skip(4);                 // digit language
        stream.Skip(4);                 // first tab offset
        stream.Skip(4);                 // hotkey prefix
        LeadingMargin = stream.F32();
        TrailingMargin = stream.F32();

        double tracking = stream.F32();
        if (tracking is > 0 and < 100) Tracking = tracking;
    }
}

/// <summary>
/// An EMF+ image: a bitmap in a device-independent form, an encoded file, or a metafile.
/// </summary>
/// <remarks>
/// <para>
/// <b>The encoded case costs no codec and the native one costs no decode.</b> A bitmap whose
/// <c>BitmapDataType</c> is "compressed" is a whole PNG, JPEG or GIF file and goes through as
/// <see cref="RasterImage.Encoded"/>, exactly as an SVG's <c>data:</c> image and a WMF's DIB do.
/// A bitmap whose type is "pixel" is raw samples with a stride and a pixel format, which is
/// arithmetic rather than a decode — the same answer the EMF side reached for the transparent-blit
/// idiom, and reached once for both.
/// </para>
/// <para>
/// A metafile image is a whole WMF or EMF nested inside the EMF+ stream. It is recognised and
/// reported rather than replayed; see the note in <c>TODO.md</c>.
/// </para>
/// </remarks>
internal sealed class EmfPlusImage : EmfPlusObject
{
    /// <summary>1 for a bitmap, 2 for a metafile.</summary>
    public int Type { get; private set; }

    /// <summary>The bitmap's width in pixels, where the record states one.</summary>
    public int Width { get; private set; }

    /// <summary>The bitmap's height in pixels.</summary>
    public int Height { get; private set; }

    /// <summary>The drawable image, or null when nothing could be made of the record.</summary>
    public RasterImage? Image { get; private set; }

    /// <summary>True when the record held a metafile rather than a bitmap.</summary>
    public bool IsMetafile => Type == 2;

    /// <summary>Reads an image object.</summary>
    /// <param name="stream">The cursor, positioned at the image's version field.</param>
    public void Read(EmfPlusStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        stream.Skip(4);                 // version
        Type = stream.I32();

        if (Type == 2)
        {
            stream.Skip(8);             // metafile type and size
            return;
        }

        if (Type != 1) return;

        int width = stream.I32();
        int height = stream.I32();
        int stride = stream.I32();
        uint format = stream.U32();
        uint dataType = stream.U32();

        if (dataType != 0 || width <= 0)
        {
            // A whole image file follows: PNG, JPEG, GIF or BMP. Nothing here decodes it.
            ReadOnlySpan<byte> encoded = stream.Rest();
            if (encoded.IsEmpty) return;

            Image = RasterImage.Encoded(encoded.ToArray());

            if (Svg.EncodedImageSize.Read(encoded) is { } size)
            {
                Width = size.Width;
                Height = size.Height;
            }

            return;
        }

        if (Native(stream, width, height, stride, format) is not { } pixels) return;

        Image = pixels.Image;
        Width = pixels.Width;
        Height = pixels.Height;
    }

    /// <summary>
    /// Turns a GDI+ native bitmap into straight RGBA.
    /// </summary>
    /// <remarks>
    /// Rows run top-down, unlike a DIB's, and the pixel format's second byte is its bit depth
    /// ([MS-EMFPLUS] 2.1.1.25). Only the two depths a producer actually writes are read: 32 bits
    /// with an alpha channel and 24 without. The rest answer null and the record falls back to
    /// drawing nothing, which is honest — a wrongly unpacked bitmap is worse than a missing one.
    /// </remarks>
    private static DeviceIndependentBitmap.Pixels? Native(
        EmfPlusStream stream, int width, int height, int stride, uint format)
    {
        int depth = (int)((format >> 8) & 0xFF);
        if (depth is not (24 or 32)) return null;
        if (width is <= 0 or > 20000 || height is <= 0 or > 20000) return null;

        int bytes = depth / 8;
        if (stride < width * bytes) stride = width * bytes;
        if ((long)stride * height > stream.Remaining) return null;

        bool premultiplied = (format & 0x00080000) != 0;
        ReadOnlySpan<byte> data = stream.Take(stride * height);
        byte[] rgba = new byte[width * height * 4];

        for (int y = 0; y < height; y++)
        {
            ReadOnlySpan<byte> row = data[(y * stride)..];

            for (int x = 0; x < width; x++)
            {
                int from = x * bytes;
                int to = ((y * width) + x) * 4;

                byte b = row[from];
                byte g = row[from + 1];
                byte r = row[from + 2];
                byte a = bytes == 4 ? row[from + 3] : (byte)0xFF;

                if (premultiplied && a is > 0 and < 255)
                {
                    r = (byte)Math.Min(255, r * 255 / a);
                    g = (byte)Math.Min(255, g * 255 / a);
                    b = (byte)Math.Min(255, b * 255 / a);
                }

                rgba[to] = r;
                rgba[to + 1] = g;
                rgba[to + 2] = b;
                rgba[to + 3] = a;
            }
        }

        return new DeviceIndependentBitmap.Pixels(rgba, width, height);
    }
}

/// <summary>
/// An EMF+ region: a binary tree of rectangles, paths and set operations.
/// </summary>
/// <remarks>
/// <para>
/// <b>The representation is the clip's, and for the clip's reason.</b> A region is held as a union
/// of rectangles intersected with a list of arbitrary paths — the same pair
/// <see cref="MetafileClip"/> keeps — because that is what a sink which only intersects can
/// replay exactly. Intersection and rectangular exclusion stay exact in that form; union of two
/// rectangle sets stays exact too, since overlapping rectangles are still their own union under
/// the non-zero rule.
/// </para>
/// <para>
/// Symmetric difference and complement are not expressible and are marked
/// <see cref="IsApproximate"/> rather than approximated silently. LibreOffice has real polygon
/// arithmetic here (<c>emfpregion.cxx</c> calls <c>solvePolygonOperationXor</c>) and gets them
/// exactly; this is the one place the port gives something up.
/// </para>
/// </remarks>
internal sealed class EmfPlusRegion : EmfPlusObject
{
    /// <summary>How deep a region tree may nest before it is refused.</summary>
    private const int MaxDepth = 64;

    /// <summary>The union of rectangles the region covers, or null for the whole plane.</summary>
    public List<DocRect>? Rectangles { get; private set; }

    /// <summary>Arbitrary shapes the region is additionally narrowed by.</summary>
    public List<GraphicsPath> Shapes { get; } = [];

    /// <summary>True when an operation could not be honoured exactly.</summary>
    public bool IsApproximate { get; private set; }

    /// <summary>True when the region covers everything.</summary>
    public bool IsInfinite => Rectangles is null && Shapes.Count == 0;

    /// <summary>True when the region covers nothing.</summary>
    public bool IsEmpty => Rectangles is { Count: 0 };

    /// <summary>Reads a region object.</summary>
    /// <param name="stream">The cursor, positioned at the region's version field.</param>
    /// <param name="map">Turns a point in the file's coordinates into a document point.</param>
    public void Read(EmfPlusStream stream, Func<double, double, DocPoint> map)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(map);

        stream.Skip(4);                 // version
        stream.Skip(4);                 // node count, which the tree itself makes redundant

        EmfPlusRegion node = Node(stream, map, 0);

        Rectangles = node.Rectangles;
        Shapes.AddRange(node.Shapes);
        IsApproximate = node.IsApproximate;
    }

    private static EmfPlusRegion Node(EmfPlusStream stream, Func<double, double, DocPoint> map, int depth)
    {
        EmfPlusRegion region = new();

        if (depth >= MaxDepth || stream.Failed)
        {
            region.IsApproximate = true;
            return region;
        }

        uint kind = stream.U32();

        switch (kind)
        {
            case 1:                     // and
            case 2:                     // or
            case 3:                     // xor
            case 4:                     // exclude
            case 5:                     // complement
            {
                EmfPlusRegion left = Node(stream, map, depth + 1);
                EmfPlusRegion right = Node(stream, map, depth + 1);
                return Combine(left, kind, right);
            }

            case 0x10000000:            // a rectangle
            {
                float x = stream.F32();
                float y = stream.F32();
                float w = stream.F32();
                float h = stream.F32();

                region.Rectangles = [DocRect.FromCorners(map(x, y), map(x + w, y + h))];
                return region;
            }

            case 0x10000001:            // a path
            {
                int length = stream.I32();
                int at = stream.Offset;

                stream.Skip(4);         // the path's own version
                int points = stream.I32();
                uint flags = stream.U32();

                if (EmfPlusPath.Read(stream, points, flags, withTypes: true) is { } path)
                {
                    region.Shapes.Add(path.ToPath(map));
                }

                if (length > 0) stream.SeekTo((long)at + length);
                return region;
            }

            case 0x10000002:            // empty
                region.Rectangles = [];
                return region;

            case 0x10000003:            // infinite
                return region;

            default:
                region.IsApproximate = true;
                return region;
        }
    }

    private static EmfPlusRegion Combine(EmfPlusRegion left, uint mode, EmfPlusRegion right)
    {
        EmfPlusRegion result = new() { IsApproximate = left.IsApproximate || right.IsApproximate };

        switch (mode)
        {
            case 1:                     // and: exact, because intersection is what the clip does
                result.Rectangles = Intersect(left.Rectangles, right.Rectangles);
                result.Shapes.AddRange(left.Shapes);
                result.Shapes.AddRange(right.Shapes);
                return result;

            case 2:                     // or
                if (left.IsInfinite || right.IsInfinite) return result;

                if (left.Shapes.Count == 0 && right.Shapes.Count == 0
                    && left.Rectangles is not null && right.Rectangles is not null)
                {
                    // Overlapping rectangles are still their own union under the non-zero rule,
                    // so no arithmetic is needed and none is lost.
                    result.Rectangles = [.. left.Rectangles, .. right.Rectangles];
                    return result;
                }

                result.IsApproximate = true;
                result.Rectangles = left.Rectangles;
                result.Shapes.AddRange(left.Shapes);
                return result;

            case 4:                     // exclude: left minus right
                if (right.Shapes.Count == 0 && right.Rectangles is { Count: > 0 })
                {
                    List<DocRect> from = left.Rectangles ?? [Infinite];
                    foreach (DocRect cut in right.Rectangles) from = Subtract(from, cut);

                    result.Rectangles = from;
                    result.Shapes.AddRange(left.Shapes);
                    return result;
                }

                if (right.IsEmpty)
                {
                    result.Rectangles = left.Rectangles;
                    result.Shapes.AddRange(left.Shapes);
                    return result;
                }

                result.IsApproximate = true;
                result.Rectangles = left.Rectangles;
                result.Shapes.AddRange(left.Shapes);
                return result;

            default:                    // xor and complement
                result.IsApproximate = true;
                result.Rectangles = mode == 5 ? right.Rectangles : left.Rectangles;
                result.Shapes.AddRange(mode == 5 ? right.Shapes : left.Shapes);
                return result;
        }
    }

    private static readonly DocRect Infinite = new(
        Core.Units.Length.FromEmu(-1L << 40),
        Core.Units.Length.FromEmu(-1L << 40),
        Core.Units.Length.FromEmu(1L << 41),
        Core.Units.Length.FromEmu(1L << 41));

    private static List<DocRect>? Intersect(List<DocRect>? left, List<DocRect>? right)
    {
        if (left is null) return right;
        if (right is null) return left;

        List<DocRect> result = [];

        foreach (DocRect a in left)
        {
            foreach (DocRect b in right)
            {
                Core.Units.Length x = Core.Units.Length.Max(a.Left, b.Left);
                Core.Units.Length y = Core.Units.Length.Max(a.Top, b.Top);
                Core.Units.Length r = Core.Units.Length.Min(a.Right, b.Right);
                Core.Units.Length s = Core.Units.Length.Min(a.Bottom, b.Bottom);

                if (r > x && s > y) result.Add(new DocRect(x, y, r - x, s - y));
            }
        }

        return result;
    }

    private static List<DocRect> Subtract(List<DocRect> from, DocRect cut)
    {
        List<DocRect> result = [];

        foreach (DocRect rect in from)
        {
            Core.Units.Length x = Core.Units.Length.Max(rect.Left, cut.Left);
            Core.Units.Length y = Core.Units.Length.Max(rect.Top, cut.Top);
            Core.Units.Length r = Core.Units.Length.Min(rect.Right, cut.Right);
            Core.Units.Length s = Core.Units.Length.Min(rect.Bottom, cut.Bottom);

            if (r <= x || s <= y)
            {
                result.Add(rect);
                continue;
            }

            if (y > rect.Top) result.Add(new DocRect(rect.Left, rect.Top, rect.Width, y - rect.Top));
            if (s < rect.Bottom) result.Add(new DocRect(rect.Left, s, rect.Width, rect.Bottom - s));
            if (x > rect.Left) result.Add(new DocRect(rect.Left, y, x - rect.Left, s - y));
            if (r < rect.Right) result.Add(new DocRect(r, y, rect.Right - r, s - y));
        }

        return result;
    }

    /// <summary>The region as one path, for a record that fills it.</summary>
    /// <remarks>
    /// The rectangles and the shapes are different operands — a union and an intersection — so a
    /// single path can only carry one of them. The caller fills the shapes clipped to the
    /// rectangles, which is what keeps the pair exact.
    /// </remarks>
    public GraphicsPath? RectanglePath()
    {
        if (Rectangles is null) return null;

        GraphicsPath path = new();
        foreach (DocRect rect in Rectangles) MetafileGeometry.AddRectangle(path, rect);
        return path;
    }
}
