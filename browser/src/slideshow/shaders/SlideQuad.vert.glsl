#version 300 es

in vec2 a_unit;
uniform vec4 u_rect;
uniform mat3 u_transform;
uniform vec2 u_canvas;
uniform float u_yDirection;
out vec2 v_texCoord;

void main() {
    vec3 at = u_transform * vec3(u_rect.xy + a_unit * u_rect.zw, 1.0);
    gl_Position = vec4(at.x / u_canvas.x * 2.0 - 1.0,
        (1.0 - at.y / u_canvas.y * 2.0) * u_yDirection, 0.0, 1.0);
    v_texCoord = a_unit;
}
