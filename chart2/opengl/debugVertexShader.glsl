attribute vec3 vPosition;
uniform vec4 vColor;
varying vec4 fragmentColor;

void main()
{
    gl_Position = vec4(vPosition, 1);
}

