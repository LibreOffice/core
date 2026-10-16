#!/usr/bin/env python3
import subprocess, sys
BANK="/home/user/odsgap-work/bank"
def txt(pdf,p):
    return subprocess.run(["pdftotext","-f",str(p),"-l",str(p),pdf,"-"],capture_output=True).stdout.decode("utf-8","replace")
stem=sys.argv[1]; n=int(sys.argv[2])
o=f"{BANK}/ours-head2/{stem}.pdf"; r=f"{BANK}/ref/{stem}.pdf"
for p in range(1,n+1):
    a,b=txt(o,p),txt(r,p)
    if a.split()!=b.split():
        print("first differing page:",p)
        print("--- ours:"); print("\n".join(a.splitlines()[:8]))
        print("--- ref:");  print("\n".join(b.splitlines()[:8]))
        break
else:
    print("no difference in the first",n,"pages")
