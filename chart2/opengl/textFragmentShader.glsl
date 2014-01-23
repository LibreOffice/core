uniform sampler2D TextTex;
varying vec2 vTexCoord;
void main()
{
    gl_FragColor = vec4(texture2D(TextTex, vTexCoord).rgba);
}
