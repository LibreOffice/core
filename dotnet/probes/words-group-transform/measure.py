import sys, numpy as np
from PIL import Image

DPI = 100.0

def boxes(png):
    im = np.asarray(Image.open(png).convert("RGB")).astype(int)
    out = {}
    for name, (r, g, b) in (("red", (255, 0, 0)), ("blue", (0, 0, 255)),
                            ("gold", (255, 204, 0))):
        m = ((abs(im[:, :, 0] - r) < 60) & (abs(im[:, :, 1] - g) < 60)
             & (abs(im[:, :, 2] - b) < 60))
        if not m.any():
            out[name] = None
            continue
        ys, xs = np.nonzero(m)
        out[name] = tuple(round(v * 72.0 / DPI, 1) for v in
                          (xs.min(), ys.min(), xs.max() - xs.min() + 1, ys.max() - ys.min() + 1))
    return out

for png in sys.argv[1:]:
    bs = boxes(png)
    print(png.rsplit("/", 1)[-1])
    for name, box in bs.items():
        print(f"   {name:5s} " + ("absent" if box is None else
              f"left={box[0]:7.1f} top={box[1]:7.1f} w={box[2]:7.1f} h={box[3]:7.1f}"))
