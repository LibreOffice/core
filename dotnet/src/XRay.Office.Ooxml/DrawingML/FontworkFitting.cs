using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// Fits a body's glyph outlines to a Fontwork shape's own outlines.
/// </summary>
/// <remarks>
/// <para>
/// A port of <c>svx/source/customshapes/EnhancedCustomShapeFontWork.cxx</c>, which is where a
/// warped body stops being text. The shape's geometry is rendered to polylines and then read as
/// <em>rails</em>: an odd number of them means each one is a baseline to lay a line of text along,
/// an even number means they pair up into envelopes and each glyph's points are mapped between the
/// two. <c>textArchUp</c>, <c>textArchDown</c>, <c>textCircle</c> and <c>textButton</c> are the
/// baseline kind; the other twenty are envelopes.
/// </para>
/// <para>
/// <strong>What the two kinds do with the font size is the surprising part and it is measurable on
/// the page.</strong> An envelope normalises the text's own ink box to the unit square before
/// mapping it, so the stated size cancels out entirely and the warped text fills the shape however
/// small the run was; only the baseline kind keeps the size, and then only by shrinking it until
/// the text fits along the curve. That is why the reference draws WordArt several times larger
/// than the run it came from.
/// </para>
/// <para>
/// Everything here works in hundredths of a millimetre, the draw layer's unit, for the reasons
/// <see cref="FontworkPoint"/> gives.
/// </para>
/// </remarks>
internal static class FontworkFitting
{
    /// <summary>EMUs per hundredth of a millimetre.</summary>
    private const double EmuPerUnit = 360.0;

    /// <summary>The whole fit, from a preset and a body to one path in shape coordinates.</summary>
    public static GraphicsPath? Fit(
        FontworkPreset preset,
        IReadOnlyList<double> adjustments,
        DocSize box,
        IReadOnlyList<string> lines,
        OpenTypeFace face,
        Length fontSize,
        FontworkAlignment alignment,
        FontworkVerticalAdjust verticalAdjust,
        bool keepsFontSize)
    {
        double width = box.Width.Emu / EmuPerUnit;
        double height = box.Height.Emu / EmuPerUnit;

        IReadOnlyList<IReadOnlyList<FontworkPoint>> rails =
            FontworkGeometry.Outlines(preset, adjustments, width, height);

        if (rails.Count == 0) return null;

        bool singleLineMode = (rails.Count & 1) != 0;
        int areaCount = singleLineMode ? rails.Count : rails.Count >> 1;
        if (areaCount == 0) return null;

        List<List<string>> areas = Distribute(lines, areaCount, out int maxParagraphs);
        if (areas.Count == 0) return null;

        double stated = fontSize.Emu / EmuPerUnit;
        (double scaling, double verticalScaling) =
            Scaling(areas, rails, singleLineMode, face, stated, height, maxParagraphs, keepsFontSize);

        double lineHeight = Math.Truncate(
            keepsFontSize ? verticalScaling * stated : height / maxParagraphs * scaling);

        if (lineHeight <= 0) return null;

        List<TextArea> laidOut = [];
        foreach (List<string> area in areas)
        {
            laidOut.Add(Compose(
                area, face, lineHeight, maxParagraphs, alignment, verticalAdjust, scaling, keepsFontSize));
        }

        GraphicsPath path = new();
        int rail = 0;

        foreach (TextArea area in laidOut)
        {
            if (singleLineMode)
            {
                if (rail >= rails.Count) break;
                AlongOneRail(area, rails[rail++], alignment, scaling, keepsFontSize, path);
            }
            else
            {
                if (rail + 1 >= rails.Count) break;
                BetweenTwoRails(area, rails[rail++], rails[rail++], path);
            }
        }

        return path.Commands.Count > 0 ? path : null;
    }

    /// <summary>
    /// Deals the lines out over the text areas, front-loading them evenly.
    /// </summary>
    /// <remarks><c>InitializeFontWorkData</c>, <c>EnhancedCustomShapeFontWork.cxx:150-172</c>.</remarks>
    private static List<List<string>> Distribute(
        IReadOnlyList<string> lines, int areaCount, out int maxParagraphs)
    {
        int left = lines.Count;
        maxParagraphs = ((left - 1) / areaCount) + 1;

        List<List<string>> areas = [];
        int next = 0;
        int remaining = areaCount;

        while (left > 0 && remaining > 0)
        {
            int here = ((left - 1) / remaining) + 1;
            List<string> area = [];
            for (int i = 0; i < here && next < lines.Count; i++) area.Add(lines[next++]);

            areas.Add(area);
            left -= here;
            remaining--;
        }

        return areas;
    }

    /// <summary>
    /// The horizontal scaling factor, and the vertical one the arch family shrinks by.
    /// </summary>
    /// <remarks>
    /// <c>CalculateHorizontalScalingFactor</c>, <c>EnhancedCustomShapeFontWork.cxx:195-287</c>. The
    /// factor is the tightest ratio of a rail's length to the text that has to lie along it. Where
    /// the font size is kept, the reference walks the size down one unit of 1/100 mm at a time until
    /// the ratio reaches one; the walk is reproduced by solving it, which is the same answer because
    /// our width measurement is exactly linear in the size.
    /// </remarks>
    private static (double Horizontal, double Vertical) Scaling(
        List<List<string>> areas,
        IReadOnlyList<IReadOnlyList<FontworkPoint>> rails,
        bool singleLineMode,
        OpenTypeFace face,
        double stated,
        double height,
        int maxParagraphs,
        bool keepsFontSize)
    {
        // The width of every paragraph at an em of one, so the search below is a multiplication.
        double tightest = double.MaxValue;
        bool defined = false;
        int rail = 0;

        foreach (List<string> area in areas)
        {
            if (rail >= rails.Count) break;

            double railWidth = ArcLength(rails[rail++]);
            if (!singleLineMode)
            {
                if (rail >= rails.Count) break;
                railWidth += ArcLength(rails[rail++]);
                railWidth /= 2.0;
            }

            foreach (string paragraph in area)
            {
                double unit = Width(face, paragraph, 1.0);
                if (unit <= 0) continue;

                double ratio = railWidth / unit;
                if (!defined || ratio < tightest)
                {
                    tightest = ratio;
                    defined = true;
                }
            }
        }

        if (!defined) return (1.0, 1.0);

        if (!keepsFontSize)
        {
            double measured = height / maxParagraphs;
            return (measured > 0 ? tightest / measured : 1.0, 1.0);
        }

        // `tightest` is the em size at which the text exactly fills the rail, so the loop's exit
        // condition -- scaling factor at or above one -- is "size at or below that".
        double size = Math.Min(Math.Truncate(stated), Math.Truncate(tightest));
        if (size < 1) size = 1;

        double scaling = size > 0 ? tightest / size : 1.0;
        double vertical = size > 1 && stated > 0 ? size / stated : 1.0;
        return (scaling, vertical);
    }

    /// <summary>Lays one text area's paragraphs out as outlines, and aligns them.</summary>
    /// <remarks>
    /// <c>GetTextAreaOutline</c> and the alignment arm of <c>GetFontWorkOutline</c>,
    /// <c>EnhancedCustomShapeFontWork.cxx:289-624</c>. The <c>TextFitToSize</c> arm is not
    /// reproduced: it scales each paragraph to the width of the area they are all measured into, so
    /// it is the identity whenever a text area holds one paragraph, which is every warped body in
    /// the corpus.
    /// </remarks>
    private static TextArea Compose(
        List<string> lines,
        OpenTypeFace face,
        double lineHeight,
        int maxParagraphs,
        FontworkAlignment alignment,
        FontworkVerticalAdjust verticalAdjust,
        double scaling,
        bool keepsFontSize)
    {
        double offset = maxParagraphs > lines.Count ? lineHeight / 2 : 0;
        List<Paragraph> paragraphs = [];
        Box areaBox = Box.Empty;

        foreach (string line in lines)
        {
            List<Character> characters = Characters(face, line, lineHeight, offset);
            Box paragraphBox = Box.Empty;
            foreach (Character character in characters) paragraphBox = paragraphBox.Union(character.Bounds);

            paragraphs.Add(new Paragraph(characters, paragraphBox));
            areaBox = paragraphBox.IsEmpty
                ? areaBox.IsEmpty ? new Box(0, 0, 0, lineHeight) : areaBox.Grown(lineHeight)
                : areaBox.Union(paragraphBox);

            offset += lineHeight;
        }

        // Horizontal alignment is applied by moving the outlines, which is what the reference does
        // and what the fit below then partly undoes for the multi-line arch case.
        double share = alignment switch
        {
            FontworkAlignment.Centre => 0.5,
            FontworkAlignment.Right => 1.0,
            _ => 0,
        };

        // Where the font size is kept, the lines are also moved off the curve by half a line each,
        // which is how the reference gets a two-line arch's text to sit where MS Office puts it:
        // `shape.cxx:864-874` sets the vertical anchor from the preset — bottom for `textArchUp`
        // and `textCircle`, top for `textArchDown`, centre for the rest — and
        // `EnhancedCustomShapeFontWork.cxx:566-570` turns that into this offset. The fit then
        // subtracts it again along the curve's normal, which is what `nHAlignMove` carries.
        double alignMove = keepsFontSize
            ? lineHeight * (verticalAdjust switch
            {
                FontworkVerticalAdjust.Bottom => -0.5,
                FontworkVerticalAdjust.Top => 0.5,
                _ => 0.0,
            }) * (paragraphs.Count - 1)
            : 0;

        for (int i = 0; i < paragraphs.Count; i++)
        {
            Paragraph paragraph = paragraphs[i];
            if (paragraph.Bounds.IsEmpty) continue;

            double across = 0;
            if (share > 0)
            {
                double available = keepsFontSize
                    ? (scaling * areaBox.Width) - paragraph.Bounds.Width
                    : areaBox.Width - paragraph.Bounds.Width;

                across = Math.Truncate(available * share);
            }

            if (across == 0 && alignMove == 0) continue;

            paragraphs[i] = paragraph.Moved(across, alignMove);
        }

        return new TextArea(paragraphs, areaBox, alignMove);
    }

    /// <summary>
    /// Lays a text area's characters along one rail, each rotated to the rail's local direction.
    /// </summary>
    /// <remarks>
    /// The fallback arm of <c>FitTextOutlinesToShapeOutlines</c>
    /// (<c>EnhancedCustomShapeFontWork.cxx:975-1017</c>): every line uses the same curve, and a
    /// character is placed rigidly — rotated about its own centre and moved onto the chord between
    /// the two points its ink box spans. The newer arm above it offsets each further line onto a
    /// parallel curve, and applies only when one text area holds more than one line, which no
    /// warped body in the corpus does.
    /// </remarks>
    private static void AlongOneRail(
        TextArea area,
        IReadOnlyList<FontworkPoint> rail,
        FontworkAlignment alignment,
        double scaling,
        bool keepsFontSize,
        GraphicsPath into)
    {
        if (rail.Count < 2 || area.Bounds.IsEmpty) return;

        double left = area.Bounds.Left;
        double width = area.Bounds.Width;
        if (keepsFontSize) width *= scaling;
        if (width <= 0) return;

        // How many halves of the spare curve go before the text: none for a left-aligned line, one
        // for a centred one, two for a right-aligned one, and "do not align at all" for justified.
        int halves = alignment switch
        {
            FontworkAlignment.Left => 0,
            FontworkAlignment.Centre => 1,
            FontworkAlignment.Right => 2,
            _ => -1,
        };

        if (area.Paragraphs.Count > 1 && halves >= 0)
        {
            AlongParallelRails(area, rail, halves, scaling, into);
            return;
        }

        double[] distances = Distances(rail);

        foreach (Paragraph paragraph in area.Paragraphs)
        {
            double centreY = paragraph.Bounds.CentreY;
            double normal = (area.Bounds.Height / 2.0) + area.Bounds.Top - centreY;

            foreach (Character character in paragraph.Characters)
            {
                if (character.Bounds.IsEmpty) continue;

                double m1 = (character.Bounds.Left - left) / width;
                double m2 = (character.Bounds.Right - left) / width;

                Place(character, PointAt(rail, distances, m1), PointAt(rail, distances, m2),
                      centreY, normal, into);
            }
        }
    }

    /// <summary>
    /// Lays several lines along one rail by offsetting each onto a curve parallel to it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>EnhancedCustomShapeFontWork.cxx:801-970</c>, the arm added for PowerPoint-shaped
    /// Fontwork. The older arm below puts every line on the <em>same</em> curve and stretches the
    /// letters apart to reach it, which for a two-line arch draws both lines through each other.
    /// This one walks the rail's normals outward by the line's own distance from the middle, which
    /// makes a longer or shorter curve for each line, and re-fits the line to that.
    /// </para>
    /// <para>
    /// Only ever reached by <c>textArchUp</c>, <c>textArchDown</c>, <c>textCircle</c> and
    /// <c>textButton</c> — the four presets whose geometry is an odd number of rails — and only
    /// when one of them holds more than one line. On the corpus that is one shape, the
    /// <c>Automation / Autonomy</c> dial label on two slides of
    /// <c>FAAAIandtheArtandScienceofV&amp;Vfinal.pptx</c>.
    /// </para>
    /// </remarks>
    private static void AlongParallelRails(
        TextArea area,
        IReadOnlyList<FontworkPoint> rail,
        int halves,
        double scaling,
        GraphicsPath into)
    {
        int count = rail.Count;
        FontworkPoint[] normals = new FontworkPoint[count];

        for (int i = 0; i < count; i++)
        {
            FontworkPoint before = rail[i == 0 ? i : i - 1];
            FontworkPoint after = rail[i == count - 1 ? i : i + 1];
            double dx = after.X - before.X;
            double dy = after.Y - before.Y;
            double length = Math.Sqrt((dx * dx) + (dy * dy));

            // Scaled by 1024 and truncated to whole units, as the reference's `Point` arithmetic
            // does, so that a long curve's offsets round the same way.
            normals[i] = length > 0
                ? new FontworkPoint(Math.Truncate(dy * 1024 / length), Math.Truncate(-dx * 1024 / length))
                : default;
        }

        FontworkPoint[] parallel = new FontworkPoint[count];
        double[] distances = new double[count];

        foreach (Paragraph paragraph in area.Paragraphs)
        {
            if (paragraph.Bounds.IsEmpty) continue;

            double centreY = paragraph.Bounds.CentreY;
            double offset = (area.Bounds.Height / 2.0) + area.Bounds.Top - centreY - area.AlignMove;

            double total = 0;
            for (int i = 0; i < count; i++)
            {
                parallel[i] = new FontworkPoint(
                    rail[i].X + (normals[i].X * offset / 1024.0),
                    rail[i].Y + (normals[i].Y * offset / 1024.0));

                if (i > 0)
                {
                    double dx = parallel[i].X - parallel[i - 1].X;
                    double dy = parallel[i].Y - parallel[i - 1].Y;
                    total += Math.Sqrt((dx * dx) + (dy * dy));
                }

                distances[i] = total;
            }

            if (total > 0)
            {
                for (int i = 0; i < count; i++) distances[i] /= total;
            }

            double paragraphWidth = paragraph.Bounds.Width;

            // How much of the curve goes before the text. Negative means the line is longer than
            // the curve, and then it is laid along the whole of it rather than shrunk.
            double before = total > paragraphWidth ? halves * (total - paragraphWidth) / 2 : -1;

            // The horizontal alignment was hacked into the outlines' coordinates; undo it here so
            // the position along the curve can be worked out from the line's own extent.
            double hack = ((scaling * area.Bounds.Width) - paragraphWidth) * halves / 2;

            foreach (Character character in paragraph.Characters)
            {
                if (character.Bounds.IsEmpty) continue;

                double x1 = character.Bounds.Left - area.Bounds.Left - hack;
                double x2 = character.Bounds.Right - area.Bounds.Left - hack;

                double m1 = paragraphWidth == 0 ? 0 : x1 / paragraphWidth;
                double m2 = paragraphWidth == 0 ? 0 : x2 / paragraphWidth;

                if (before >= 0 && total > 0)
                {
                    m1 = ((m1 * paragraphWidth) + before) / total;
                    m2 = ((m2 * paragraphWidth) + before) / total;
                }

                if (m1 < 0) m1 = 0;
                if (m2 < 0) m2 = 0;

                Place(character, PointAt(parallel, distances, m1), PointAt(parallel, distances, m2),
                      centreY, area.AlignMove, into);
            }
        }
    }

    /// <summary>
    /// Turns one character onto the chord between two points and moves it there.
    /// </summary>
    /// <param name="character">The character to place.</param>
    /// <param name="from">Where its left edge lands on the curve.</param>
    /// <param name="to">Where its right edge lands.</param>
    /// <param name="centreY">
    /// The y its line was laid out about, which is the axis it turns around and the height the
    /// move is measured from.
    /// </param>
    /// <param name="normal">How far along the chord's normal it then moves.</param>
    /// <param name="into">The path to append it to.</param>
    private static void Place(
        Character character,
        FontworkPoint from,
        FontworkPoint to,
        double centreY,
        double normal,
        GraphicsPath into)
    {
        double vx = to.Y - from.Y;
        double vy = -(to.X - from.X);
        double midX = from.X + ((to.X - from.X) * 0.5);
        double midY = from.Y + ((to.Y - from.Y) * 0.5);

        double angle = Math.Atan2(-vx, -vy);
        double length = Math.Sqrt((vx * vx) + (vy * vy));
        if (length == 0) return;

        vx = vx / length * normal;
        vy = vy / length * normal;

        double pivotX = character.Bounds.CentreX;
        character
            .Rotated(pivotX, centreY, Math.Sin(angle), Math.Cos(angle))
            .Moved(midX + vx - pivotX, midY + vy - centreY)
            .AppendTo(into);
    }

    /// <summary>
    /// Maps a text area's characters into the envelope between two rails.
    /// </summary>
    /// <remarks>
    /// The even arm of <c>FitTextOutlinesToShapeOutlines</c>
    /// (<c>EnhancedCustomShapeFontWork.cxx:1021-1085</c>). Each point of each glyph is placed by
    /// two fractions: how far along the rails its x sits in the text's own ink box, and how far
    /// between the two rails its y sits. Extra points are inserted first, wherever the glyph
    /// crosses one of the rails' own vertices, so that a straight stem still follows a bend.
    /// </remarks>
    private static void BetweenTwoRails(
        TextArea area,
        IReadOnlyList<FontworkPoint> first,
        IReadOnlyList<FontworkPoint> second,
        GraphicsPath into)
    {
        if (first.Count < 2 || second.Count < 2 || area.Bounds.IsEmpty) return;

        double left = area.Bounds.Left;
        double top = area.Bounds.Top;
        double width = area.Bounds.Width;
        double height = area.Bounds.Height;
        if (width <= 0 || height <= 0) return;

        double[] distances1 = Distances(first);
        double[] distances2 = Distances(second);

        foreach (Paragraph paragraph in area.Paragraphs)
        {
            foreach (Character character in paragraph.Characters)
            {
                foreach (List<FontworkPoint> contour in character.Flattened())
                {
                    List<FontworkPoint> points = contour;
                    points = InsertMissing(distances1, left, width, points);
                    points = InsertMissing(distances2, left, width, points);
                    if (points.Count == 0) continue;

                    bool started = false;
                    foreach (FontworkPoint point in points)
                    {
                        double x = (point.X - left) / width;
                        double y = (point.Y - top) / height;

                        FontworkPoint a = PointAt(first, distances1, x);
                        FontworkPoint b = PointAt(second, distances2, x);

                        DocPoint mapped = At(
                            a.X + ((b.X - a.X) * y),
                            a.Y + ((b.Y - a.Y) * y));

                        if (started) into.LineTo(mapped);
                        else
                        {
                            into.MoveTo(mapped);
                            started = true;
                        }
                    }

                    into.Close();
                }
            }
        }
    }

    /// <summary>
    /// Splits a glyph's segments wherever it crosses one of a rail's own parameter values.
    /// </summary>
    /// <remarks><c>InsertMissingOutlinePoints</c>, <c>EnhancedCustomShapeFontWork.cxx:669-720</c>.</remarks>
    private static List<FontworkPoint> InsertMissing(
        double[] distances, double left, double width, List<FontworkPoint> points)
    {
        if (points.Count == 0 || width == 0 || distances.Length == 0) return points;

        List<FontworkPoint> result = new(points.Count + 8);
        double last = 0;

        for (int i = 0; i < points.Count; i++)
        {
            FontworkPoint point = points[i];
            double here = (point.X - left) / width;

            if (i > 0)
            {
                double? crossing = here > last ? FirstAbove(distances, last, here)
                    : here < last ? LastBelow(distances, here, last)
                    : null;

                if (crossing is { } at)
                {
                    FontworkPoint previous = points[i - 1];
                    double t = (at - last) / (here - last);
                    result.Add(new FontworkPoint(
                        previous.X + ((point.X - previous.X) * t),
                        previous.Y + ((point.Y - previous.Y) * t)));
                    here = at;
                }
            }

            result.Add(point);
            last = here;
        }

        return result;
    }

    /// <summary>The first rail parameter strictly between the two, or null.</summary>
    private static double? FirstAbove(double[] distances, double after, double before)
    {
        foreach (double distance in distances)
        {
            if (distance <= after) continue;
            return distance < before ? distance : null;
        }

        return null;
    }

    /// <summary>The last rail parameter strictly between the two, or null.</summary>
    private static double? LastBelow(double[] distances, double after, double before)
    {
        for (int i = distances.Length - 1; i >= 0; i--)
        {
            if (distances[i] >= before) continue;
            return distances[i] > after ? distances[i] : null;
        }

        return null;
    }

    /// <summary>A rail's cumulative length at each of its points, normalised to end at one.</summary>
    /// <remarks><c>CalcDistances</c>, <c>EnhancedCustomShapeFontWork.cxx:649-667</c>.</remarks>
    private static double[] Distances(IReadOnlyList<FontworkPoint> rail)
    {
        double[] distances = new double[rail.Count];
        double total = 0;

        for (int i = 1; i < rail.Count; i++)
        {
            double dx = rail[i].X - rail[i - 1].X;
            double dy = rail[i].Y - rail[i - 1].Y;
            total += Math.Sqrt((dx * dx) + (dy * dy));
            distances[i] = total;
        }

        if (total > 0)
        {
            for (int i = 0; i < distances.Length; i++) distances[i] /= total;
        }

        return distances;
    }

    /// <summary>The point a fraction of the way along a rail, by arc length.</summary>
    /// <remarks><c>GetPoint</c>, <c>EnhancedCustomShapeFontWork.cxx:724-750</c>.</remarks>
    private static FontworkPoint PointAt(
        IReadOnlyList<FontworkPoint> rail, double[] distances, double fraction)
    {
        if (rail.Count <= 1) return default;

        int index = LowerBound(distances, fraction);
        bool past = index >= distances.Length;
        if (past) index = distances.Length - 1;

        FontworkPoint point = rail[index];
        if (index == 0 || past || Math.Abs(distances[index] - fraction) <= 1e-12) return point;

        double previous = distances[index - 1];
        double span = distances[index] - previous;
        if (span == 0) return point;

        double t = (fraction - previous) / span;
        FontworkPoint before = rail[index - 1];
        return new FontworkPoint(
            before.X + ((point.X - before.X) * t),
            before.Y + ((point.Y - before.Y) * t));
    }

    /// <summary>The first index whose value is at or above the target.</summary>
    private static int LowerBound(double[] values, double target)
    {
        int low = 0;
        int high = values.Length;

        while (low < high)
        {
            int middle = (low + high) / 2;
            if (values[middle] < target) low = middle + 1;
            else high = middle;
        }

        return low;
    }

    /// <summary>A polyline's length.</summary>
    private static double ArcLength(IReadOnlyList<FontworkPoint> polyline)
    {
        double total = 0;
        for (int i = 1; i < polyline.Count; i++)
        {
            double dx = polyline[i].X - polyline[i - 1].X;
            double dy = polyline[i].Y - polyline[i - 1].Y;
            total += Math.Sqrt((dx * dx) + (dy * dy));
        }

        return total;
    }

    /// <summary>How wide a string sets at the given em size, from the face's own advances.</summary>
    private static double Width(OpenTypeFace face, string text, double emSize)
    {
        double units = 0;
        foreach (char character in text) units += face.AdvanceForCharacter(character);

        return units * emSize / face.UnitsPerEm;
    }

    /// <summary>One glyph outline per character of a line, placed at the pen.</summary>
    /// <remarks>
    /// The reference asks the device for the outlines of the whole string and gets one polygon per
    /// character back, positioned by the layout's own array. Here the pen walks the face's advances
    /// instead: Fontwork sets no kerning and every warped body in the corpus is Latin, so the two
    /// agree glyph for glyph.
    /// </remarks>
    private static List<Character> Characters(
        OpenTypeFace face, string text, double emSize, double verticalOffset)
    {
        List<Character> characters = [];
        Length em = Length.FromEmu((long)Math.Round(emSize * EmuPerUnit));
        double scale = 1.0 / EmuPerUnit;

        // ALIGN_TOP: the reference's outlines are measured from the top of the line, not from the
        // baseline. Every point of every character moves by the same amount, so this cancels out of
        // the fit entirely; it is here so that a multi-line body stacks the way the reference does.
        double baseline = face.Horizontal.Ascender * emSize / face.UnitsPerEm;
        double pen = 0;

        foreach (char character in text)
        {
            ushort glyph = face.Characters.GlyphFor(character);
            GraphicsPath? outline = GlyphOutlines.Of(face, glyph, em);

            if (outline is { Commands.Count: > 0 })
            {
                List<Segment> segments = [];
                Box bounds = Box.Empty;

                foreach (PathCommand command in outline.Commands)
                {
                    FontworkPoint point = new(
                        (command.Point.X.Emu * scale) + pen,
                        (command.Point.Y.Emu * scale) + baseline + verticalOffset);
                    FontworkPoint control1 = new(
                        (command.Control1.X.Emu * scale) + pen,
                        (command.Control1.Y.Emu * scale) + baseline + verticalOffset);
                    FontworkPoint control2 = new(
                        (command.Control2.X.Emu * scale) + pen,
                        (command.Control2.Y.Emu * scale) + baseline + verticalOffset);

                    segments.Add(new Segment(command.Verb, point, control1, control2));

                    if (command.Verb == PathVerb.Close) continue;

                    bounds = bounds.Including(point);
                    if (command.Verb != PathVerb.CubicTo) continue;

                    bounds = bounds.Including(control1).Including(control2);
                }

                characters.Add(new Character(segments, bounds));
            }

            pen += face.AdvanceOf(glyph) * emSize / face.UnitsPerEm;
        }

        return characters;
    }

    /// <summary>A point in the shape's coordinates, back in EMUs.</summary>
    private static DocPoint At(double x, double y) => new(
        Length.FromEmu((long)Math.Round(x * EmuPerUnit)),
        Length.FromEmu((long)Math.Round(y * EmuPerUnit)));

    /// <summary>One command of a character's outline, in Fontwork coordinates.</summary>
    private readonly record struct Segment(
        PathVerb Verb, FontworkPoint Point, FontworkPoint Control1, FontworkPoint Control2)
    {
        /// <summary>This command with every point moved through the same map.</summary>
        public Segment Mapped(Func<FontworkPoint, FontworkPoint> map)
            => new(Verb, map(Point), map(Control1), map(Control2));
    }

    /// <summary>One character's outline and the box its points fill.</summary>
    private sealed record Character(IReadOnlyList<Segment> Segments, Box Bounds)
    {
        /// <summary>The same character rotated about a point, as <c>Polygon::Rotate</c> rotates.</summary>
        /// <remarks><c>tools/source/generic/poly.cxx:1475-1489</c>.</remarks>
        public Character Rotated(double centreX, double centreY, double sine, double cosine)
            => Mapped(point =>
            {
                double x = point.X - centreX;
                double y = point.Y - centreY;
                return new FontworkPoint(
                    (cosine * x) + (sine * y) + centreX,
                    -((sine * x) - (cosine * y) - centreY));
            });

        /// <summary>The same character translated.</summary>
        public Character Moved(double dx, double dy)
            => Mapped(point => new FontworkPoint(point.X + dx, point.Y + dy));

        /// <summary>Appends the character to a path, in EMUs.</summary>
        public void AppendTo(GraphicsPath path)
        {
            foreach (Segment segment in Segments)
            {
                switch (segment.Verb)
                {
                    case PathVerb.MoveTo: path.MoveTo(At(segment.Point.X, segment.Point.Y)); break;
                    case PathVerb.LineTo: path.LineTo(At(segment.Point.X, segment.Point.Y)); break;
                    case PathVerb.CubicTo:
                        path.CubicTo(
                            At(segment.Control1.X, segment.Control1.Y),
                            At(segment.Control2.X, segment.Control2.Y),
                            At(segment.Point.X, segment.Point.Y));
                        break;
                    case PathVerb.Close: path.Close(); break;
                    default: break;
                }
            }
        }

        /// <summary>The character's contours as polylines, for the envelope mapping.</summary>
        public List<List<FontworkPoint>> Flattened()
        {
            List<List<FontworkPoint>> contours = [];
            List<FontworkPoint> current = [];
            FontworkPoint pen = default;

            foreach (Segment segment in Segments)
            {
                switch (segment.Verb)
                {
                    case PathVerb.MoveTo:
                        if (current.Count > 1) contours.Add(current);
                        current = [segment.Point];
                        pen = segment.Point;
                        break;

                    case PathVerb.LineTo:
                        current.Add(segment.Point);
                        pen = segment.Point;
                        break;

                    case PathVerb.CubicTo:
                        FontworkFlattening.Append(pen, segment.Control1, segment.Control2, segment.Point, current);
                        pen = segment.Point;
                        break;

                    case PathVerb.Close:
                        if (current.Count > 1)
                        {
                            current.Add(current[0]);
                            contours.Add(current);
                        }

                        current = [];
                        break;

                    default:
                        break;
                }
            }

            if (current.Count > 1) contours.Add(current);
            return contours;
        }

        private Character Mapped(Func<FontworkPoint, FontworkPoint> map)
        {
            List<Segment> mapped = new(Segments.Count);
            foreach (Segment segment in Segments) mapped.Add(segment.Mapped(map));

            return new Character(mapped, Bounds.Mapped(map));
        }
    }

    /// <summary>One line of text: its characters and the box they fill together.</summary>
    /// <remarks>
    /// <see cref="Bounds"/> is where the line was <em>laid out</em>, not where its characters
    /// currently are. The reference moves the outlines for horizontal and vertical alignment and
    /// never updates <c>rParagraph.aBoundRect</c> behind them, and the fit then relies on that: it
    /// subtracts the same alignment back out of each character's position, so a bounding box that
    /// had moved with them would subtract it twice.
    /// </remarks>
    private sealed record Paragraph(IReadOnlyList<Character> Characters, Box Bounds)
    {
        /// <summary>The same paragraph's characters translated, leaving its box where it was.</summary>
        public Paragraph Moved(double dx, double dy)
        {
            List<Character> moved = new(Characters.Count);
            foreach (Character character in Characters) moved.Add(character.Moved(dx, dy));

            return new Paragraph(moved, Bounds);
        }
    }

    /// <summary>The lines fitted to one rail or one pair of rails.</summary>
    /// <param name="Paragraphs">Its lines.</param>
    /// <param name="Bounds">The box they were laid out in, before any alignment moved them.</param>
    /// <param name="AlignMove">
    /// How far the vertical anchor moved them, which the fit takes back out along the curve's
    /// normal. <c>FWTextArea::nHAlignMove</c>.
    /// </param>
    private sealed record TextArea(
        IReadOnlyList<Paragraph> Paragraphs, Box Bounds, double AlignMove = 0);

    /// <summary>
    /// A bounding box with VCL's <c>tools::Rectangle</c> arithmetic.
    /// </summary>
    /// <remarks>
    /// Its edges are inclusive, so its width is one unit more than the span between them, and an
    /// unset box unions to whatever it meets. Both matter: the whole fit divides by these widths.
    /// </remarks>
    private readonly record struct Box(double Left, double Top, double Right, double Bottom)
    {
        /// <summary>A box holding nothing.</summary>
        public static Box Empty => new(double.NaN, double.NaN, double.NaN, double.NaN);

        /// <summary>Whether the box has been given a point yet.</summary>
        public bool IsEmpty => double.IsNaN(Left);

        /// <summary>Its width, counting both edges.</summary>
        public double Width => IsEmpty ? 0 : Right - Left + 1;

        /// <summary>Its height, counting both edges.</summary>
        public double Height => IsEmpty ? 0 : Bottom - Top + 1;

        /// <summary>The middle of its horizontal span.</summary>
        public double CentreX => IsEmpty ? 0 : Left + Math.Truncate((Right - Left) / 2);

        /// <summary>The middle of its vertical span.</summary>
        public double CentreY => IsEmpty ? 0 : Top + Math.Truncate((Bottom - Top) / 2);

        /// <summary>This box grown to hold a point.</summary>
        public Box Including(FontworkPoint point) => IsEmpty
            ? new Box(point.X, point.Y, point.X, point.Y)
            : new Box(
                Math.Min(Left, point.X),
                Math.Min(Top, point.Y),
                Math.Max(Right, point.X),
                Math.Max(Bottom, point.Y));

        /// <summary>This box grown to hold another.</summary>
        public Box Union(Box other)
        {
            if (other.IsEmpty) return this;
            if (IsEmpty) return other;

            return new Box(
                Math.Min(Left, other.Left),
                Math.Min(Top, other.Top),
                Math.Max(Right, other.Right),
                Math.Max(Bottom, other.Bottom));
        }

        /// <summary>This box with its bottom pushed down, which is what an empty line does to it.</summary>
        public Box Grown(double height) => IsEmpty ? this : this with { Bottom = Bottom + height };

        /// <summary>The box of this one's corners under a map.</summary>
        public Box Mapped(Func<FontworkPoint, FontworkPoint> map)
        {
            if (IsEmpty) return this;

            Box result = Empty;
            result = result.Including(map(new FontworkPoint(Left, Top)));
            result = result.Including(map(new FontworkPoint(Right, Top)));
            result = result.Including(map(new FontworkPoint(Right, Bottom)));
            return result.Including(map(new FontworkPoint(Left, Bottom)));
        }
    }
}
