using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Rendering.Fills;

/// <summary>
/// The arithmetic both backends share when they draw a <see cref="MeshPaint"/>.
/// </summary>
/// <remarks>
/// Very little, and deliberately: a mesh is the one paint kind both backends state natively
/// with no decomposition at all — PDF as a <c>/ShadingType 4</c> triangle stream, Skia as
/// <c>SkVertices</c> — so the only thing worth sharing is the reading of the triples, which
/// must agree about which triangles are real. A generated mesh can name an index that is not
/// there (a boundary of one point fans into nothing), and the two backends silently differing
/// about whether that triangle exists is exactly the class of bug the shared band
/// decomposition in <see cref="Gradients"/> exists to prevent.
/// </remarks>
internal static class Meshes
{
    /// <summary>
    /// The triangles that name three real, distinct vertices, in the order stated.
    /// </summary>
    /// <remarks>
    /// Degenerate triples — a repeated index, or one out of range — are dropped rather than
    /// treated as an error. A mesh is generated from a document's own geometry, so a boundary
    /// with a doubled point produces one, and a zero-area triangle paints nothing in either
    /// backend anyway.
    /// </remarks>
    public static IEnumerable<MeshTriangle> Valid(MeshPaint mesh)
    {
        ArgumentNullException.ThrowIfNull(mesh);

        int count = mesh.Vertices.Count;

        foreach (MeshTriangle triangle in mesh.Triangles)
        {
            if ((uint)triangle.A >= (uint)count) continue;
            if ((uint)triangle.B >= (uint)count) continue;
            if ((uint)triangle.C >= (uint)count) continue;
            if (triangle.A == triangle.B || triangle.B == triangle.C || triangle.A == triangle.C) continue;

            yield return triangle;
        }
    }

    /// <summary>Whether any vertex a real triangle names is less than fully opaque.</summary>
    /// <remarks>
    /// Asked of the vertices the mesh actually <em>draws</em> rather than of every vertex it
    /// holds, because the PDF backend pays a whole transparency group for the answer being
    /// yes and a dropped triangle must not cost that.
    /// </remarks>
    public static bool Fades(MeshPaint mesh)
    {
        ArgumentNullException.ThrowIfNull(mesh);

        foreach (MeshTriangle triangle in Valid(mesh))
        {
            if (mesh.Vertices[triangle.A].Colour.A != 255) return true;
            if (mesh.Vertices[triangle.B].Colour.A != 255) return true;
            if (mesh.Vertices[triangle.C].Colour.A != 255) return true;
        }

        return false;
    }

    /// <summary>
    /// The box the drawn triangles cover, or null when none of them are real.
    /// </summary>
    /// <remarks>
    /// PDF needs it as the <c>/Decode</c> range a type 4 shading's packed coordinates are
    /// expanded through, so it must come from the same triangle set the stream carries or a
    /// vertex quantises to the wrong place.
    /// </remarks>
    public static DocRect? Bounds(MeshPaint mesh)
    {
        ArgumentNullException.ThrowIfNull(mesh);

        bool any = false;
        long left = 0, top = 0, right = 0, bottom = 0;

        foreach (MeshTriangle triangle in Valid(mesh))
        {
            Include(mesh.Vertices[triangle.A].Position);
            Include(mesh.Vertices[triangle.B].Position);
            Include(mesh.Vertices[triangle.C].Position);
        }

        return any
            ? new DocRect(
                Length.FromEmu(left), Length.FromEmu(top),
                Length.FromEmu(right - left), Length.FromEmu(bottom - top))
            : null;

        void Include(DocPoint point)
        {
            if (!any)
            {
                left = right = point.X.Emu;
                top = bottom = point.Y.Emu;
                any = true;
                return;
            }

            left = Math.Min(left, point.X.Emu);
            right = Math.Max(right, point.X.Emu);
            top = Math.Min(top, point.Y.Emu);
            bottom = Math.Max(bottom, point.Y.Emu);
        }
    }

    /// <summary>The average of every drawn vertex's colour, for a backend that can only take one.</summary>
    /// <remarks>
    /// A mesh reaches a stroke or a glyph run the same way a gradient does — as one flat
    /// colour — and the mean of its vertices is the nearest single colour to what it paints.
    /// </remarks>
    public static Colour Average(MeshPaint mesh)
    {
        ArgumentNullException.ThrowIfNull(mesh);

        long r = 0, g = 0, b = 0, a = 0;
        int n = 0;

        foreach (MeshTriangle triangle in Valid(mesh))
        {
            foreach (int index in (ReadOnlySpan<int>)[triangle.A, triangle.B, triangle.C])
            {
                Colour colour = mesh.Vertices[index].Colour;
                r += colour.R;
                g += colour.G;
                b += colour.B;
                a += colour.A;
                n++;
            }
        }

        if (n == 0) return Colour.Transparent;

        return new Colour((byte)(r / n), (byte)(g / n), (byte)(b / n), (byte)(a / n));
    }
}
