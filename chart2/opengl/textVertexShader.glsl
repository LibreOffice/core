attribute vec3 vPosition;
uniform mat4 MVP;
attribute vec2 texCoord;
varying vec2 vTexCoord;
void main()
{
    gl_Position =  MVP * vec4(vPosition, 1);
    vTexCoord = texCoord;
}
