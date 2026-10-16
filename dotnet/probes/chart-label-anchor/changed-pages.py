"""Which pages of a document's rendering the change moved at all, by rasterising both
sides at 72 dpi and hashing. A page whose raster is identical cannot have moved its score."""
import hashlib, os, subprocess, sys, tempfile

SP = "/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/charts2"


def pdf(base, key):
    d = os.path.join(SP, base, key)
    f = [x for x in os.listdir(d) if x.endswith(".pdf")][0]
    return os.path.join(d, f)


for key in sys.argv[1:]:
    b, a = pdf("render-before", key), pdf("render-after", key)
    differ = []
    with tempfile.TemporaryDirectory() as t:
        for tag, p in (("b", b), ("a", a)):
            subprocess.run(["pdftoppm", "-r", "72", "-gray", "-png", p, os.path.join(t, tag)],
                           capture_output=True)
        for n in sorted(x for x in os.listdir(t) if x.startswith("b-")):
            pb = os.path.join(t, n)
            pa = os.path.join(t, "a-" + n[2:])
            if not os.path.exists(pa):
                differ.append(n[2:] + "(missing)")
                continue
            hb = hashlib.md5(open(pb, "rb").read()).hexdigest()
            ha = hashlib.md5(open(pa, "rb").read()).hexdigest()
            if hb != ha:
                differ.append(n[2:].split(".")[0])
    print(key, "pages differing:", differ, flush=True)
