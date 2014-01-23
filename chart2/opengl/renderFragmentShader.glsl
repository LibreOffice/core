uniform sampler2D RenderTex;
varying vec2 vTexCoord;

void main()
{
    gl_FragColor = vec4(texture2D(RenderTex, vTexCoord).rgb, 1.0);
}
