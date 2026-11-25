#version 300 es
precision mediump float;

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texCoord;

uniform float u_aspect;
uniform float u_eyeDistance;

out vec2 v_texturePosition;
out vec3 v_normal;

void main(void) {
	// Perspective projection, where the z = 0 plane fills the viewport. The
	// x axis is in units of half slide height, so rotations keep the shape.
	float w = (u_eyeDistance - a_position.z) / u_eyeDistance;
	float depth = clamp(-a_position.z / 20.0, -1.0, 1.0);
	gl_Position = vec4(a_position.x / u_aspect, a_position.y, depth * w, w);
	v_texturePosition = a_texCoord;
	v_normal = a_normal;
}
