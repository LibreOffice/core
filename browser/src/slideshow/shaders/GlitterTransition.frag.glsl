#version 300 es
precision mediump float;

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D leavingSlideTexture;
uniform sampler2D enteringSlideTexture;

in vec2 v_texturePosition;
in vec3 v_normal;
in float v_angle;

out vec4 outColor;

void main() {
	vec3 lightVector = vec3(0.0, 0.0, 1.0);
	float light = dot(lightVector, v_normal);
	vec4 fragment;
	if (v_angle < M_PI)
		fragment = texture(leavingSlideTexture, v_texturePosition);
	else
		fragment = texture(enteringSlideTexture, v_texturePosition);
	vec4 black = vec4(0.0, 0.0, 0.0, fragment.a);
	outColor = mix(black, fragment, max(light, 0.0));
}
