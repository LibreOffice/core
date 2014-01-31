#version 120

varying vec4 fragmentColor;

void main()
{
    vec2 p = gl_PointCoord * 2.0 - vec2(1.0); // (0,0) in the center
    if (abs(p.x) < abs(p.y))
        discard;

    gl_FragColor = fragmentColor;
}
