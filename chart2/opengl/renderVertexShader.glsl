attribute vec4 vPosition;
attribute vec2 texCoord;
varying vec2 vTexCoord;

void main()
{
    gl_Position =  vPosition;
    vTexCoord = texCoord;
}
