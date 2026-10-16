#!/usr/bin/env python3
import pathlib, subprocess, sys
CLI='/home/user/wt-chartinner/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli'
for a in sys.argv[1:]:
    src=pathlib.Path(a)
    r=subprocess.run([CLI,'render',str(src),'--format','pdf','--outdir','/home/user/r110-work/orend'],
                     capture_output=True, text=True,
                     env={'XRAY_OFFICE_CHART_TRACE':'1','PATH':'/usr/bin:/bin','HOME':'/root'})
    seen=set(); i=0
    for line in r.stderr.splitlines():
        if not line.startswith('TRACE'): continue
        if line in seen: continue
        seen.add(line)
        d=dict(kv.split('=',1) for kv in line.split()[1:] if '=' in kv)
        ou=[float(v) for v in d['outer'].strip('()').split(',')]
        ar=[float(v) for v in d['area'].strip('()').split(',')]
        sq_o=min(ou[2],ou[3]); sq_a=min(ar[2],ar[3])
        print(f'{src.name}\t{i}\t{d["kind"]}{"/ring" if d["rings"]=="True" else ""}'
              f'\touter ({ou[0]:.2f},{ou[1]:.2f},{ou[2]:.2f},{ou[3]:.2f})'
              f'\tarea ({ar[0]:.2f},{ar[1]:.2f},{ar[2]:.2f},{ar[3]:.2f})'
              f'\tshrink {sq_a/sq_o if sq_o else 0:.4f}')
        i+=1
