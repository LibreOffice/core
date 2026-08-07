using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// The hatch each <c>ST_PresetPatternVal</c> is drawn as.
/// </summary>
/// <remarks>
/// <para>
/// A direct port of <c>oox/inc/drawingml/hatchmap.hxx</c>, whose own comment calls it a
/// "best-effort mapping; we do not support all the styles in core". That inexactness is the
/// point rather than a caveat: DrawingML's presets are 8×8 monochrome bitmaps and LibreOffice
/// draws none of them, so the reference renderings this project is measured against are these
/// approximations. Synthesising the real bitmaps would move us *away* from the reference on
/// every one of the corpus's sixty-five pattern fills.
/// </para>
/// <para>
/// The whole table is here rather than the five presets the corpus uses, because a table is
/// what the source is and picking five out of it would be a special case wearing a lookup's
/// clothes. The names not listed — <c>ltDnDiag</c> and <c>dkDnDiag</c> share a row, as do
/// several others — share a row upstream too.
/// </para>
/// </remarks>
public static class DrawingHatchPresets
{
    /// <summary>
    /// The hatch a preset names, or null when the token is not one of the fifty-four.
    /// </summary>
    /// <remarks>
    /// Returns the geometry only. The caller supplies the colours, because a
    /// <c>a:fgClr</c>/<c>a:bgClr</c> pair may name theme colours or a placeholder that only the
    /// family's own reader can resolve.
    /// </remarks>
    /// <param name="preset">The <c>a:pattFill/@prst</c> token.</param>
    public static (HatchKind Kind, Length Distance, double Angle)? Hatch(string? preset)
        => preset switch
        {
            "pct5" => (HatchKind.OneWay, Mm100(250), Deg(45)),
            "pct10" => (HatchKind.OneWay, Mm100(200), Deg(45)),
            "pct20" or "dashUpDiag" => (HatchKind.OneWay, Mm100(150), Deg(45)),
            "pct25" => (HatchKind.TwoWay, Mm100(200), Deg(45)),
            "pct30" => (HatchKind.TwoWay, Mm100(175), Deg(45)),
            "pct40" or "weave" => (HatchKind.TwoWay, Mm100(150), Deg(45)),
            "pct50" => (HatchKind.TwoWay, Mm100(125), Deg(45)),
            "pct60" => (HatchKind.ThreeWay, Mm100(150), Deg(45)),
            "pct70" => (HatchKind.ThreeWay, Mm100(125), Deg(45)),
            "pct75" => (HatchKind.ThreeWay, Mm100(100), Deg(45)),
            "pct80" => (HatchKind.ThreeWay, Mm100(75), Deg(45)),
            "pct90" => (HatchKind.ThreeWay, Mm100(50), Deg(45)),
            "horz" or "wave" => (HatchKind.OneWay, Mm100(100), Deg(0)),
            "vert" => (HatchKind.OneWay, Mm100(100), Deg(90)),
            "ltHorz" or "narHorz" => (HatchKind.OneWay, Mm100(50), Deg(0)),
            "ltVert" or "narVert" => (HatchKind.OneWay, Mm100(50), Deg(90)),
            "dkHorz" => (HatchKind.OneWay, Mm100(25), Deg(0)),
            "dkVert" => (HatchKind.OneWay, Mm100(25), Deg(90)),
            "dashHorz" => (HatchKind.OneWay, Mm100(150), Deg(0)),
            "dashVert" => (HatchKind.OneWay, Mm100(150), Deg(90)),
            "cross" or "lgGrid" => (HatchKind.TwoWay, Mm100(100), Deg(0)),
            "dnDiag" or "wdDnDiag" => (HatchKind.OneWay, Mm100(100), Deg(135)),
            "upDiag" or "wdUpDiag" => (HatchKind.OneWay, Mm100(100), Deg(45)),
            "ltDnDiag" or "dkDnDiag" => (HatchKind.OneWay, Mm100(50), Deg(135)),
            "ltUpDiag" or "dkUpDiag" => (HatchKind.OneWay, Mm100(50), Deg(45)),
            "dashDnDiag" => (HatchKind.OneWay, Mm100(150), Deg(135)),
            "diagCross" => (HatchKind.TwoWay, Mm100(100), Deg(45)),
            "smCheck" => (HatchKind.TwoWay, Mm100(50), Deg(45)),
            "lgCheck" => (HatchKind.TwoWay, Mm100(100), Deg(45)),
            "smGrid" => (HatchKind.TwoWay, Mm100(50), Deg(0)),
            "dotGrid" => (HatchKind.TwoWay, Mm100(400), Deg(0)),
            "smConfetti" => (HatchKind.OneWay, Mm100(200), Deg(60)),
            "lgConfetti" => (HatchKind.OneWay, Mm100(100), Deg(60)),
            "horzBrick" => (HatchKind.TwoWay, Mm100(300), Deg(0)),
            "diagBrick" or "dotDmnd" => (HatchKind.TwoWay, Mm100(300), Deg(45)),
            "solidDmnd" or "openDmnd" => (HatchKind.TwoWay, Mm100(100), Deg(45)),
            "plaid" => (HatchKind.ThreeWay, Mm100(200), Deg(90)),
            "sphere" => (HatchKind.ThreeWay, Mm100(100), Deg(0)),
            "divot" => (HatchKind.ThreeWay, Mm100(400), Deg(45)),
            "shingle" => (HatchKind.OneWay, Mm100(200), Deg(135)),
            "trellis" => (HatchKind.TwoWay, Mm100(75), Deg(45)),
            "zigZag" => (HatchKind.OneWay, Mm100(75), Deg(0)),
            _ => null,
        };

    private static Length Mm100(long value) => Length.FromMm100(value);

    private static double Deg(double degrees) => degrees * Math.PI / 180.0;
}
