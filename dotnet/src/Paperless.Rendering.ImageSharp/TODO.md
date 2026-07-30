# Paperless.Rendering.ImageSharp — TODO

An alternative fully-managed raster backend. **Off by default** and nothing depends on it.

## Read this first

`SixLabors.ImageSharp` 4.0.0 ships an MSBuild target that **fails the build** unless a
purchased Six Labors licence key is present — verified, not theoretical. The package
reference is therefore gated behind `EnableImageSharpBackend`, default false.

So this project is a placeholder with a rationale, not work in progress. Only pick it up if:

- someone holds a Six Labors licence, **and**
- there is a real deployment that cannot ship SkiaSharp's native binaries — a trimmed
  fully-managed build, or a platform with no Skia build.

Otherwise use the Skia backend and leave this alone.

## If it is implemented

- [ ] `IDrawingSink` over ImageSharp
- [ ] Paths, gradients, transparency groups
- [ ] Glyph rendering from glyph IDs and positions — note ImageSharp's text support is
      weaker here than Skia's, and this is the part most likely to be unsatisfying
- [ ] **Separate golden images.** Output will not be pixel-identical to the Skia backend:
      the two rasterise antialiasing differently. Golden images are per-backend.
