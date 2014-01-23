attribute vec3 vPosition;
uniform mat4 MVP;
uniform vec4 vColor;
varying vec4 fragmentColor;

void main()
{
    gl_Position =  MVP * vec4(vPosition, 1);
    fragmentColor = vColor;
}
