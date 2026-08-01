using Paperless.Core.Graphics;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// The device context a metafile's records mutate: the mapping, the clip, the selected
/// objects and the drawing modes.
/// </summary>
/// <remarks>
/// <para>
/// <b>The seam gives a decoder none of this on purpose.</b> <c>IDrawingSink</c> is stateless
/// per draw call — every call carries its own paint — because that keeps a recorded command
/// list reorderable and cacheable. WMF, EMF and EMF+ are the opposite: stateful command streams
/// where a <c>Polygon</c> record carries points and nothing else, and the pen, brush, fill rule,
/// clip and mapping that decide what it looks like were all set by earlier records. Something
/// has to hold that state, and it is the same state in all three formats, so it is built once
/// here.
/// </para>
/// <para>
/// <b>Save and restore are not the sink's.</b> GDI's <c>SaveDC</c> pushes the mapping, the
/// selected objects and the modes; <c>RestoreDC</c> can pop several levels at once, and with a
/// <em>positive</em> argument restores a numbered state rather than a relative one — WMF uses
/// both spellings. The sink's <c>Save</c>/<c>Restore</c> is a clip-and-transform stack and
/// cannot express either. Ported from <c>MtfTools::Push</c> and <c>Pop</c>
/// (<c>emfio/source/reader/mtftools.cxx:3043-3160</c>).
/// </para>
/// <para>
/// <b>The object table is not saved.</b> GDI objects belong to the device, not to the saved
/// state, so a <c>RestoreDC</c> after a <c>CreateBrush</c> leaves the brush created and its
/// handle taken. Selecting one copies it into the context by value, which is what makes
/// deleting a selected object — legal, and common — harmless.
/// </para>
/// </remarks>
public sealed class MetafileDeviceContext
{
    private readonly List<SavedState> _stack = [];

    /// <summary>The window/viewport arithmetic in force.</summary>
    public MetafileMapping Mapping { get; } = new();

    /// <summary>The clipping region in force.</summary>
    public MetafileClip Clip { get; private set; } = new();

    /// <summary>The handle table. Shared by every saved state, because GDI objects are the device's.</summary>
    public GraphicsObjectTable Objects { get; } = new();

    /// <summary>The selected pen.</summary>
    public MetafilePen Pen { get; set; } = MetafilePen.Default;

    /// <summary>The selected brush.</summary>
    public MetafileBrush Brush { get; set; } = MetafileBrush.Default;

    /// <summary>The selected font.</summary>
    public MetafileFont Font { get; set; } = MetafileFont.Default;

    /// <summary>The selected palette, which colour references index into.</summary>
    public MetafilePalette Palette { get; set; } = MetafilePalette.Empty;

    /// <summary>
    /// True once the file has selected a brush of its own.
    /// </summary>
    /// <remarks>
    /// Until it has, GDI fills with the background colour rather than with the default white
    /// brush — <c>MtfTools::UpdateFillStyle</c> (<c>mtftools.cxx:1368-1372</c>), which cites
    /// i57205. A decoder that starts with white instead paints white boxes over everything a
    /// background-coloured shape was supposed to reveal.
    /// </remarks>
    public bool IsBrushSelected { get; set; }

    /// <summary>The colour text is drawn in.</summary>
    public Colour TextColour { get; set; } = Colour.Black;

    /// <summary>The colour behind text, hatches and dash gaps when the mode is opaque.</summary>
    public Colour BackgroundColour { get; set; } = Colour.White;

    /// <summary>Whether gaps are filled with <see cref="BackgroundColour"/>.</summary>
    public BackgroundMode BackgroundMode { get; set; } = BackgroundMode.Opaque;

    /// <summary>Where a text record's point sits relative to its text.</summary>
    public TextAlignment TextAlignment { get; set; } = TextAlignment.Default;

    /// <summary>Which rule decides the inside of a self-intersecting polygon.</summary>
    public PolygonFillMode PolygonFillMode { get; set; } = PolygonFillMode.Alternate;

    /// <summary>The current position, in logical units.</summary>
    /// <remarks>
    /// Logical rather than mapped, because <c>LineTo</c> maps it afresh and a mapping record may
    /// intervene between the <c>MoveTo</c> and the <c>LineTo</c>. Keeping it mapped would freeze
    /// it under the old mapping.
    /// </remarks>
    public (double X, double Y) CurrentPosition { get; set; }

    /// <summary>True when arcs sweep clockwise. GDI's default is anticlockwise.</summary>
    public bool IsArcDirectionClockwise { get; set; }

    /// <summary>
    /// True when the current raster operation makes drawing invisible, so records still have to
    /// be read for their side effects but must not paint.
    /// </summary>
    /// <remarks>
    /// Only <c>R2_NOP</c> is modelled. The rest of GDI's sixteen binary raster operations —
    /// XOR, invert, mask — have no equivalent in a device-independent sink, and rendering them
    /// as ordinary over-paint is what LibreOffice does too.
    /// </remarks>
    public bool IsNoOperation { get; set; }

    /// <summary>How deep the save stack is.</summary>
    public int SaveDepth => _stack.Count;

    /// <summary>
    /// The brush that actually fills, accounting for a file that has selected none.
    /// </summary>
    public MetafileBrush EffectiveBrush => IsBrushSelected
        ? Brush
        : new MetafileBrush(
            BackgroundMode == Metafiles.BackgroundMode.Transparent ? BrushStyle.Null : BrushStyle.Solid,
            BackgroundColour);

    /// <summary>Which fill rule the current polygon fill mode means.</summary>
    public FillRule FillRule
        => PolygonFillMode == PolygonFillMode.Winding ? Core.Graphics.FillRule.NonZero : Core.Graphics.FillRule.EvenOdd;

    /// <summary>Pushes the whole context, as <c>SaveDC</c> does.</summary>
    public void Save() => _stack.Add(new SavedState(this));

    /// <summary>
    /// Pops the context, as <c>RestoreDC</c> does.
    /// </summary>
    /// <param name="saved">
    /// -1 for the most recent state, -n for n levels up, or a positive number naming a specific
    /// saved state. Zero is a no-op. WMF writes both spellings, sometimes in one file, and
    /// treating a positive value as a count discards states a later record still expects
    /// (<c>MtfTools::Pop</c>, <c>mtftools.cxx:3089-3106</c>).
    /// </param>
    public void Restore(int saved = -1)
    {
        if (saved == 0) return;

        int index = saved < 0 ? _stack.Count + saved : saved;

        if (index < 0)
        {
            _stack.Clear();
            return;
        }

        if (_stack.Count == 0 || index >= _stack.Count) return;

        _stack.RemoveRange(index + 1, _stack.Count - index - 1);
        _stack[^1].RestoreTo(this);
        _stack.RemoveAt(_stack.Count - 1);
    }

    /// <summary>
    /// Selects an object by handle, which changes whichever of pen, brush, font or palette it
    /// turns out to be.
    /// </summary>
    /// <returns>False when the handle names nothing, so a caller can record a diagnostic.</returns>
    public bool Select(int handle)
    {
        if (SelectStock(handle)) return true;

        switch (Objects[handle & 0xFFFF])
        {
            case MetafilePen pen:
                Pen = pen;
                return true;

            case MetafileBrush brush:
                Brush = brush;
                IsBrushSelected = true;
                return true;

            case MetafileFont font:
                Font = font;
                return true;

            case MetafilePalette palette:
                Palette = palette;
                return true;

            case MetafileRegion:
            case UnsupportedGraphicsObject:
                return true;

            default:
                return false;
        }
    }

    /// <summary>
    /// Selects one of GDI's predefined objects, which a file may name without creating.
    /// </summary>
    /// <remarks>
    /// EMF flags these with the top bit of the handle; WMF names them as bare small integers,
    /// which is indistinguishable from an ordinary handle — so this is tried only after the
    /// table has been asked, and in WMF only for handles the table does not know.
    /// </remarks>
    /// <returns>True when the handle named a stock object.</returns>
    public bool SelectStock(int handle)
    {
        const uint StockFlag = 0x80000000;
        if (((uint)handle & StockFlag) == 0) return false;

        switch ((StockObject)(handle & 0xFF))
        {
            case StockObject.WhiteBrush:
                Brush = new MetafileBrush(BrushStyle.Solid, Colour.White);
                IsBrushSelected = true;
                return true;

            case StockObject.LightGrayBrush:
                Brush = new MetafileBrush(BrushStyle.Solid, new Colour(0xC0, 0xC0, 0xC0));
                IsBrushSelected = true;
                return true;

            case StockObject.GrayBrush:
                Brush = new MetafileBrush(BrushStyle.Solid, new Colour(0x80, 0x80, 0x80));
                IsBrushSelected = true;
                return true;

            case StockObject.DarkGrayBrush:
                Brush = new MetafileBrush(BrushStyle.Solid, new Colour(0x40, 0x40, 0x40));
                IsBrushSelected = true;
                return true;

            case StockObject.BlackBrush:
                Brush = new MetafileBrush(BrushStyle.Solid, Colour.Black);
                IsBrushSelected = true;
                return true;

            case StockObject.NullBrush:
                Brush = MetafileBrush.None;
                IsBrushSelected = true;
                return true;

            case StockObject.WhitePen:
                Pen = new MetafilePen(Colour.White, Core.Units.Length.Zero);
                return true;

            case StockObject.BlackPen:
                Pen = MetafilePen.Default;
                return true;

            case StockObject.NullPen:
                Pen = MetafilePen.None;
                return true;

            case StockObject.AnsiFixedFont:
            case StockObject.SystemFixedFont:
                Font = Font with { Family = "Courier New" };
                return true;

            case StockObject.AnsiVariableFont:
                Font = Font with { Family = "Arial" };
                return true;

            default:
                return true;
        }
    }

    /// <summary>Everything <c>SaveDC</c> preserves.</summary>
    private sealed class SavedState
    {
        private readonly MetafileMapping _mapping;
        private readonly MetafileClip _clip;
        private readonly MetafilePen _pen;
        private readonly MetafileBrush _brush;
        private readonly MetafileFont _font;
        private readonly MetafilePalette _palette;
        private readonly bool _brushSelected;
        private readonly Colour _textColour;
        private readonly Colour _backgroundColour;
        private readonly BackgroundMode _backgroundMode;
        private readonly TextAlignment _textAlignment;
        private readonly PolygonFillMode _fillMode;
        private readonly (double X, double Y) _position;
        private readonly bool _arcClockwise;
        private readonly bool _noOperation;

        public SavedState(MetafileDeviceContext context)
        {
            _mapping = context.Mapping.Clone();
            _clip = context.Clip.Clone();
            _pen = context.Pen;
            _brush = context.Brush;
            _font = context.Font;
            _palette = context.Palette;
            _brushSelected = context.IsBrushSelected;
            _textColour = context.TextColour;
            _backgroundColour = context.BackgroundColour;
            _backgroundMode = context.BackgroundMode;
            _textAlignment = context.TextAlignment;
            _fillMode = context.PolygonFillMode;
            _position = context.CurrentPosition;
            _arcClockwise = context.IsArcDirectionClockwise;
            _noOperation = context.IsNoOperation;
        }

        public void RestoreTo(MetafileDeviceContext context)
        {
            context.Mapping.RestoreFrom(_mapping);
            context.Clip = _clip;
            context.Pen = _pen;
            context.Brush = _brush;
            context.Font = _font;
            context.Palette = _palette;
            context.IsBrushSelected = _brushSelected;
            context.TextColour = _textColour;
            context.BackgroundColour = _backgroundColour;
            context.BackgroundMode = _backgroundMode;
            context.TextAlignment = _textAlignment;
            context.PolygonFillMode = _fillMode;
            context.CurrentPosition = _position;
            context.IsArcDirectionClockwise = _arcClockwise;
            context.IsNoOperation = _noOperation;
        }
    }
}
