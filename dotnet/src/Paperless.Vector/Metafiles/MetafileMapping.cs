using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// The window/viewport arithmetic that turns a metafile's logical coordinates into physical
/// ones.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is the part that decides whether a picture comes out the right size and the right
/// way up.</b> A metafile states its coordinate space (the <em>window</em>) and the physical
/// extent that space is mapped onto (the <em>viewport</em>) as two independent quantities, and
/// then names a <see cref="MappingMode"/> saying how the two relate. Five of the eight modes
/// also flip the y axis. Ported from <c>MtfTools::ImplMap</c>
/// (<c>emfio/source/reader/mtftools.cxx:541-626</c>) rather than from the specification,
/// because the specification does not record which of GDI's undocumented behaviours real
/// files depend on.
/// </para>
/// <para>
/// <b>The intermediate unit is 1/100 mm and the answer is EMUs.</b> LibreOffice's whole
/// metafile pipeline works in 1/100 mm and every conversion factor in <c>ImplMap</c> is
/// expressed as "into mm100", so keeping that intermediate makes the port checkable
/// line-by-line against the original. 914400/2540 = 360 is exact, so nothing is lost turning
/// the result into EMUs — and it happens in <see cref="Emu(double)"/> and nowhere else, which
/// is the rule <c>Svg/ShimGeometry.cs</c> sets: a scale factor applied in two places is a
/// scale factor applied twice.
/// </para>
/// <para>
/// The state here is deliberately separable from the rest of the device context, because
/// <c>SaveDC</c>/<c>RestoreDC</c> save it as a unit and because the reference-device fields
/// (<see cref="ReferencePixelsX"/> and friends) are set once from a header rather than by
/// records.
/// </para>
/// </remarks>
public sealed class MetafileMapping
{
    /// <summary>EMUs per 1/100 mm. Exact: 914400/2540.</summary>
    public const long EmuPerMm100 = Length.EmuPerMm100;

    /// <summary>The world transform, applied before anything else.</summary>
    /// <remarks>
    /// WMF has no record that sets this, so it stays the identity there; EMF's
    /// <c>SetWorldTransform</c> and <c>ModifyWorldTransform</c> do. It is here rather than in
    /// the EMF reader because <c>ImplMap</c> applies it unconditionally and because a pen width
    /// and a font size have to be scaled by it too.
    /// </remarks>
    public AffineTransform World { get; set; } = AffineTransform.Identity;

    /// <summary>The mapping mode. Starts at <see cref="MappingMode.Text"/>, as GDI does.</summary>
    public MappingMode Mode { get; set; } = MappingMode.Text;

    /// <summary>
    /// True in <c>GM_ADVANCED</c>, where the world transform is the whole mapping and the
    /// window/viewport arithmetic is bypassed.
    /// </summary>
    public bool IsAdvanced { get; set; }

    /// <summary>The logical coordinate of the window's top-left corner.</summary>
    public int WindowOriginX { get; set; }

    /// <inheritdoc cref="WindowOriginX"/>
    public int WindowOriginY { get; set; }

    /// <summary>The window's width in logical units. Never zero; a zero extent is ignored.</summary>
    public int WindowExtentX { get; private set; } = 1;

    /// <inheritdoc cref="WindowExtentX"/>
    public int WindowExtentY { get; private set; } = 1;

    /// <summary>The viewport's origin, in 1/100 mm before the reference-device scale.</summary>
    public int ViewportOriginX { get; set; }

    /// <inheritdoc cref="ViewportOriginX"/>
    public int ViewportOriginY { get; set; }

    /// <summary>The viewport's width, in the units the window extent is mapped onto.</summary>
    public int ViewportExtentX { get; private set; } = 1;

    /// <inheritdoc cref="ViewportExtentX"/>
    public int ViewportExtentY { get; private set; } = 1;

    /// <summary>The reference device's width in pixels. EMF's <c>szlDevice</c>.</summary>
    public int ReferencePixelsX { get; set; } = 100;

    /// <inheritdoc cref="ReferencePixelsX"/>
    public int ReferencePixelsY { get; set; } = 100;

    /// <summary>The reference device's width in millimetres. EMF's <c>szlMillimeters</c>.</summary>
    /// <remarks>
    /// The defaults of 100 pixels to 1 mm make <c>millimetres * 100 / pixels</c> exactly one,
    /// which is what makes a WMF — which has no reference device at all — map straight into
    /// 1/100 mm. It is not a plausible device; it is the identity written as one.
    /// </remarks>
    public int ReferenceMillimetresX { get; set; } = 1;

    /// <inheritdoc cref="ReferenceMillimetresX"/>
    public int ReferenceMillimetresY { get; set; } = 1;

    /// <summary>The frame rectangle's top-left corner in 1/100 mm, subtracted from every point.</summary>
    /// <remarks>
    /// EMF's <c>rclFrame</c> need not start at the origin, and a picture whose frame starts at
    /// (500, 500) must not be drawn 5 mm down and right of where it belongs.
    /// </remarks>
    public double FrameOffsetX { get; set; }

    /// <inheritdoc cref="FrameOffsetX"/>
    public double FrameOffsetY { get; set; }

    /// <summary>True once a record or header has stated the viewport extent.</summary>
    /// <remarks>
    /// LibreOffice's <c>mbIsMapDevSet</c>. It exists because a file that never states one has
    /// to be given a default, and the default has to be distinguishable from a real value.
    /// </remarks>
    public bool IsViewportExtentSet { get; private set; }

    /// <summary>True once a record or header has stated the window extent.</summary>
    public bool IsWindowExtentSet { get; private set; }

    /// <summary>A length in 1/100 mm as an EMU length.</summary>
    /// <remarks>
    /// The one place the unit changes. Clamped because a malformed file can state extents that
    /// overflow every intermediate, and a picture drawn at 10^300 EMUs is a picture that is not
    /// drawn at all rather than a crash.
    /// </remarks>
    public static Length Emu(double mm100)
        => Length.FromEmu((long)Math.Round(Math.Clamp(mm100 * EmuPerMm100, -1e17, 1e17)));

    /// <summary>Sets the window extent, ignoring a zero one as GDI does.</summary>
    /// <remarks>
    /// Only <see cref="MappingMode.Isotropic"/> and <see cref="MappingMode.Anisotropic"/> take
    /// a window extent; in the fixed modes the scale is the mode's own and the record is a
    /// no-op (<c>MtfTools::SetWinExt</c>, <c>mtftools.cxx:2880</c>). Applying it anyway is one
    /// of the two ways a metric-mode picture ends up at an arbitrary scale.
    /// </remarks>
    public void SetWindowExtent(int width, int height)
    {
        if (width == 0 || height == 0) return;
        if (Mode is not (MappingMode.Isotropic or MappingMode.Anisotropic)) return;

        WindowExtentX = width;
        WindowExtentY = height;
        IsWindowExtentSet = true;
    }

    /// <summary>Scales the window extent by a rational factor.</summary>
    public void ScaleWindowExtent(double x, double y)
    {
        WindowExtentX = Clamp(WindowExtentX * x);
        WindowExtentY = Clamp(WindowExtentY * y);
    }

    /// <summary>Sets the viewport extent, ignoring a zero one.</summary>
    /// <param name="width">The width.</param>
    /// <param name="height">The height.</param>
    /// <param name="stated">
    /// False when the value is a guess the reader made rather than one the file gave, so that
    /// <see cref="IsViewportExtentSet"/> keeps meaning "the file said so".
    /// </param>
    public void SetViewportExtent(int width, int height, bool stated = true)
    {
        if (width == 0 || height == 0) return;

        if (Mode is MappingMode.Isotropic or MappingMode.Anisotropic)
        {
            ViewportExtentX = width;
            ViewportExtentY = height;
        }

        if (stated) IsViewportExtentSet = true;
    }

    /// <summary>Scales the viewport extent by a rational factor.</summary>
    public void ScaleViewportExtent(double x, double y)
    {
        ViewportExtentX = Clamp(ViewportExtentX * x);
        ViewportExtentY = Clamp(ViewportExtentY * y);
    }

    /// <summary>
    /// Sets the mapping mode, applying the two extents GDI derives from it.
    /// </summary>
    /// <remarks>
    /// <see cref="MappingMode.Text"/> makes the window extent equal the viewport extent unless
    /// the file has already stated one, and <see cref="MappingMode.HiMetric"/> derives it from
    /// the reference device — both from <c>MtfTools::SetMapMode</c>
    /// (<c>mtftools.cxx:2934-2953</c>). Without them a <c>SetMapMode(MM_TEXT)</c> that follows
    /// a viewport record silently rescales everything after it.
    /// </remarks>
    public void SetMode(MappingMode mode)
    {
        Mode = mode;

        if (mode == MappingMode.Text && !IsWindowExtentSet)
        {
            WindowExtentX = ViewportExtentX;
            WindowExtentY = ViewportExtentY;
        }
        else if (mode == MappingMode.HiMetric)
        {
            WindowExtentX = Clamp((double)ReferenceMillimetresX * 100);
            WindowExtentY = Clamp((double)ReferenceMillimetresY * 100);
        }
    }

    /// <summary>Maps a logical point to a document point.</summary>
    public DocPoint MapPoint(double x, double y)
    {
        (double mx, double my) = MapPointMm100(x, y);
        return new DocPoint(Emu(mx), Emu(my));
    }

    /// <summary>Maps a logical point into 1/100 mm.</summary>
    /// <remarks>
    /// The straight port of <c>MtfTools::ImplMap(const Point&amp;)</c>. Answered in 1/100 mm
    /// rather than EMUs so that callers composing several mapped values — an arc's centre and
    /// its radii, a DX array's running sum — round once at the end rather than at every step.
    /// </remarks>
    public (double X, double Y) MapPointMm100(double x, double y)
    {
        if (WindowExtentX == 0 || WindowExtentY == 0) return (0, 0);

        double px = (x * World.A) + (y * World.C) + World.E;
        double py = (x * World.B) + (y * World.D) + World.F;

        if (IsAdvanced) return (px, py);

        px -= WindowOriginX;
        py -= WindowOriginY;

        switch (Mode)
        {
            case MappingMode.LoEnglish:
                px *= Mm100PerInch / 100.0;
                py *= -Mm100PerInch / 100.0;
                break;

            case MappingMode.HiEnglish:
                px *= Mm100PerInch / 1000.0;
                py *= -Mm100PerInch / 1000.0;
                break;

            case MappingMode.Twips:
                px *= Mm100PerInch / 1440.0;
                py *= -Mm100PerInch / 1440.0;
                break;

            case MappingMode.LoMetric:
                px *= 10.0;
                py *= -10.0;
                break;

            case MappingMode.HiMetric:
                py = -py;
                break;

            default:
                if (ReferencePixelsX == 0 || ReferencePixelsY == 0) return (0, 0);

                if (Mode != MappingMode.Text)
                {
                    px = px / WindowExtentX * ViewportExtentX;
                    py = py / WindowExtentY * ViewportExtentY;
                }

                px *= DeviceScaleX;
                py *= DeviceScaleY;
                break;
        }

        px += ViewportOriginX * DeviceScaleX;
        py += ViewportOriginY * DeviceScaleY;

        return (px - FrameOffsetX, py - FrameOffsetY);
    }

    /// <summary>Maps a logical size — a distance, not a position — into 1/100 mm.</summary>
    /// <remarks>
    /// A separate path from <see cref="MapPointMm100"/> because the window origin, the viewport
    /// origin and the frame offset are translations and must not be applied to a distance.
    /// Getting that wrong makes every width and height in the picture wrong by the origin.
    /// </remarks>
    /// <param name="width">The logical width.</param>
    /// <param name="height">The logical height.</param>
    /// <param name="rotate">
    /// False to take only the world transform's scale and drop its rotation, which is what a
    /// font size needs: a rotated font is still measured along its own baseline
    /// (<c>MtfTools::ImplMap(const Size&amp;, bool)</c>, <c>mtftools.cxx:628</c>).
    /// </param>
    public (double Width, double Height) MapSizeMm100(double width, double height, bool rotate = true)
    {
        if (WindowExtentX == 0 || WindowExtentY == 0) return (0, 0);

        double w;
        double h;

        if (rotate)
        {
            w = (width * World.A) + (height * World.C);
            h = (width * World.B) + (height * World.D);
        }
        else
        {
            // The scale factors of the world transform, taken as the column norms. That is what
            // decomposing the matrix and discarding the rotation amounts to, and it is stable
            // for the degenerate matrices real files contain.
            double sx = Math.Sqrt((World.A * World.A) + (World.B * World.B));
            double sy = Math.Sqrt((World.C * World.C) + (World.D * World.D));
            if (sx == 0) sx = 1;
            if (sy == 0) sy = 1;
            w = width * sx;
            h = height * sy;
        }

        if (IsAdvanced) return (w, h);

        switch (Mode)
        {
            case MappingMode.LoEnglish:
                w *= Mm100PerInch / 100.0;
                h *= -Mm100PerInch / 100.0;
                break;

            case MappingMode.HiEnglish:
                w *= Mm100PerInch / 1000.0;
                h *= -Mm100PerInch / 1000.0;
                break;

            case MappingMode.Twips:
                w *= Mm100PerInch / 1440.0;
                h *= -Mm100PerInch / 1440.0;
                break;

            case MappingMode.LoMetric:
                w *= 10.0;
                h *= -10.0;
                break;

            case MappingMode.HiMetric:
                h = -h;
                break;

            default:
                if (ReferencePixelsX == 0 || ReferencePixelsY == 0) return (0, 0);

                if (Mode != MappingMode.Text)
                {
                    w = w / WindowExtentX * ViewportExtentX;
                    h = h / WindowExtentY * ViewportExtentY;
                }

                w *= DeviceScaleX;
                h *= DeviceScaleY;
                break;
        }

        return (w, h);
    }

    /// <summary>Maps a logical size to a document size, normalised to be positive.</summary>
    public DocSize MapSize(double width, double height, bool rotate = true)
    {
        (double w, double h) = MapSizeMm100(width, height, rotate);
        return new DocSize(Emu(Math.Abs(w)), Emu(Math.Abs(h)));
    }

    /// <summary>Maps a logical rectangle, normalising it.</summary>
    /// <remarks>
    /// Normalised because a y-flipping mapping mode turns a top-left/bottom-right pair into a
    /// bottom-left/top-right one, and every consumer downstream assumes width and height are
    /// positive.
    /// </remarks>
    public DocRect MapRect(double left, double top, double right, double bottom)
        => DocRect.FromCorners(MapPoint(left, top), MapPoint(right, bottom));

    /// <summary>
    /// Maps a scalar that is a horizontal distance — a pen width, a corner radius.
    /// </summary>
    /// <remarks>
    /// A pen width is one number but a mapping is two, so something has to decide which axis it
    /// follows. GDI, and LibreOffice after it, take the x axis
    /// (<c>MtfTools::CreateObject</c>, <c>mtftools.cxx:1027</c>), which is why a pen in a
    /// picture with wildly different x and y scales looks wrong in one of them whatever we do.
    /// </remarks>
    public Length MapWidth(double width)
    {
        (double w, _) = MapSizeMm100(width, 0);
        return Emu(Math.Abs(w));
    }

    /// <summary>True when the mapping mirrors exactly one axis, so drawing order winds backwards.</summary>
    /// <remarks>
    /// Text is the visible symptom: a single mirrored axis reverses the direction a rotated
    /// string reads in, while mirroring both is an ordinary 180-degree rotation
    /// (<c>MtfTools::DrawText</c>, <c>mtftools.cxx:2119-2136</c>).
    /// </remarks>
    public bool IsMirrored
    {
        get
        {
            (double x1, double y1) = MapSizeMm100(1, 0);
            (double x2, double y2) = MapSizeMm100(0, 1);
            return ((x1 * y2) - (y1 * x2)) < 0;
        }
    }

    /// <summary>A copy, for the save stack.</summary>
    public MetafileMapping Clone() => (MetafileMapping)MemberwiseClone();

    /// <summary>Restores every field from a saved copy.</summary>
    /// <remarks>
    /// The reference device and the frame offset are deliberately excluded: they come from the
    /// file header, not from records, and <c>RestoreDC</c> does not touch them
    /// (<c>MtfTools::Pop</c>, <c>mtftools.cxx:3089</c>).
    /// </remarks>
    public void RestoreFrom(MetafileMapping saved)
    {
        ArgumentNullException.ThrowIfNull(saved);

        World = saved.World;
        Mode = saved.Mode;
        IsAdvanced = saved.IsAdvanced;
        WindowOriginX = saved.WindowOriginX;
        WindowOriginY = saved.WindowOriginY;
        WindowExtentX = saved.WindowExtentX;
        WindowExtentY = saved.WindowExtentY;
        ViewportOriginX = saved.ViewportOriginX;
        ViewportOriginY = saved.ViewportOriginY;
        ViewportExtentX = saved.ViewportExtentX;
        ViewportExtentY = saved.ViewportExtentY;
    }

    private const double Mm100PerInch = 2540.0;

    private double DeviceScaleX => ReferencePixelsX == 0 ? 1 : ReferenceMillimetresX * 100.0 / ReferencePixelsX;

    private double DeviceScaleY => ReferencePixelsY == 0 ? 1 : ReferenceMillimetresY * 100.0 / ReferencePixelsY;

    private static int Clamp(double value)
        => (int)Math.Round(Math.Clamp(value, int.MinValue / 2.0, int.MaxValue / 2.0));
}
