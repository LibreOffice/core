import pymupdf, math
def refpts(pno, want, path='/home/user/gate-orig-r83/ref/microsoft_learn_multi_chart_examples__xlsx.pdf'):
    d=pymupdf.open(path); pg=d[pno]
    for dr in pg.get_drawings():
        if dr['type']=='s' and len(dr['items'])==want and all(i[0]=='l' for i in dr['items']):
            return [(dr['items'][0][1].x,dr['items'][0][1].y)]+[(i[2].x,i[2].y) for i in dr['items']]
def natural(at, val, d0=None, dn=None):
    n=len(at)-1; sec=[0.0]*(n+1); u=[0.0]*max(n,1)
    if d0 is not None:
        sec[0]=-0.5; xd=at[1]-at[0]; u[0]=(3.0/xd)*(((val[1]-val[0])/xd)-d0)
    for i in range(1,n):
        sig=(at[i]-at[i-1])/(at[i+1]-at[i-1]); p=sig*sec[i-1]+2.0
        sec[i]=(sig-1.0)/p
        uu=((val[i+1]-val[i])/(at[i+1]-at[i]))-((val[i]-val[i-1])/(at[i]-at[i-1]))
        u[i]=(6.0*uu/(at[i+1]-at[i-1])-sig*u[i-1])/p
    qn=un=0.0
    if dn is not None:
        qn=0.5; xd=at[n]-at[n-1]; un=(3.0/xd)*(dn-(val[n]-val[n-1])/xd)
    sec[n]=(un-qn*u[n-1])/(qn*sec[n-1]+1.0)
    for k in range(n,0,-1): sec[k-1]=sec[k-1]*sec[k]+u[k-1]
    return sec
def interp(at,val,sec,x):
    lo,hi=0,len(at)-1
    while hi-lo>1:
        m=(hi+lo)//2
        if at[m]>x: hi=m
        else: lo=m
    h=at[hi]-at[lo]
    if h==0: return val[lo]
    a=(at[hi]-x)/h; b=(x-at[lo])/h
    return a*val[lo]+b*val[hi]+(((a**3-a)*sec[lo]+(b**3-b)*sec[hi])*h*h/6.0)
def flatten_index(pts,g=20):
    last=len(pts)-1
    at=[float(i) for i in range(len(pts))]
    xs=[p[0] for p in pts]; ys=[p[1] for p in pts]
    sx=natural(at,xs); sy=natural(at,ys)
    out=[]
    for i in range(last):
        out.append(pts[i]); inc=(at[i+1]-at[i])/g
        for j in range(1,g):
            t=at[i]+inc*j
            out.append((interp(at,xs,sx,t), interp(at,ys,sy,t)))
    out.append(pts[last]); return out
def flatten_x(pts,g=20):
    last=len(pts)-1
    xs=[p[0] for p in pts]; ys=[p[1] for p in pts]
    sy=natural(xs,ys)
    out=[]
    for i in range(last):
        out.append(pts[i]); inc=(xs[i+1]-xs[i])/g
        for j in range(1,g):
            x=xs[i]+inc*j
            out.append((x, interp(xs,ys,sy,x)))
    out.append(pts[last]); return out
def flatten_clamped(pts,g=20):
    last=len(pts)-1
    at=[float(i) for i in range(len(pts))]
    xs=[p[0] for p in pts]; ys=[p[1] for p in pts]
    # clamped with the chord slope at each end
    sx=natural(at,xs, xs[1]-xs[0], xs[-1]-xs[-2]); sy=natural(at,ys, ys[1]-ys[0], ys[-1]-ys[-2])
    out=[]
    for i in range(last):
        out.append(pts[i]); inc=(at[i+1]-at[i])/g
        for j in range(1,g):
            t=at[i]+inc*j
            out.append((interp(at,xs,sx,t), interp(at,ys,sy,t)))
    out.append(pts[last]); return out
def dev(A,B): return max(math.hypot(a[0]-b[0],a[1]-b[1]) for a,b in zip(A,B))

# ---------------------------------------------------------------------------------------------
# What this file is for.
#
# 26.2.4.2 strokes a smoothed series as a flattened polyline, so its own PDF states both the
# input points (every twentieth vertex, copied through by Splines.cxx:600-606) and the expected
# output (all of them). Feeding the first back through each candidate flattening and measuring
# the distance to the second separates the hypotheses with no free parameter.
#
# Run: python3 spline-hypotheses.py       (writes the table in hypotheses.txt)

if __name__ == '__main__':
    for pno, want, what in [(0, 60, 'four-point line series, page 1'),
                            (2, 200, 'eleven-point scatter series with uneven x, page 3')]:
        R = refpts(pno, want)
        K = [R[i] for i in range(0, len(R), 20)]
        straight = []
        for i in range(len(R) - 1):
            a, b = K[i // 20], K[i // 20 + 1]
            t = (i % 20) / 20.0
            straight.append((a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t))
        straight.append(K[-1])
        print(f'{what}: {len(K)} knots, {len(R)} reference vertices')
        print(f'   parameterised by index, natural ends    {dev(flatten_index(K), R):.4f} pt')
        print(f'   parameterised by index, clamped ends    {dev(flatten_clamped(K), R):.4f} pt')
        print(f'   parameterised by x value, natural ends  {dev(flatten_x(K), R):.4f} pt')
        print(f'   no spline at all, the straight polyline {dev(straight, R):.4f} pt')
        U = 2540.0 / 72.0
        dx = [(p[0] - R[0][0]) * U for p in R]
        dy = [(p[1] - R[0][1]) * U for p in R]
        print('   every reference vertex is a whole 1/100 mm from the first, to within '
              f'{max(max(abs(v - round(v)) for v in dx), max(abs(v - round(v)) for v in dy)):.4f} '
              'of a unit')
