#version 300 es

in vec2 a_pixel;
uniform vec2 u_canvas;
uniform float u_yDirection;

void main() {
    gl_Position = vec4(a_pixel.x / u_canvas.x * 2.0 - 1.0,
        (1.0 - a_pixel.y / u_canvas.y * 2.0) * u_yDirection, 0.0, 1.0);
}
