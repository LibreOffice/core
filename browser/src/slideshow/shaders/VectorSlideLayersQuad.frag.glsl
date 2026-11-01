#version 300 es
precision mediump float;

uniform sampler2D u_texture;
uniform float u_alpha;
in vec2 v_texCoord;
out vec4 outColor;

void main() {
    outColor = texture(u_texture, v_texCoord) * u_alpha;
}
