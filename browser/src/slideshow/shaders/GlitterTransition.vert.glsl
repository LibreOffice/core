#version 300 es
precision highp float;

#define M_PI 3.1415926535897932384626433832795

in vec3 a_position;
in vec3 a_normal;
in vec3 a_center;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_sceneTransformMatrix;
uniform mat4 u_primitiveTransformMatrix;
uniform mat4 u_operationsTransformMatrix;
uniform float time;
uniform sampler2D permTexture;

out vec2 v_texturePosition;
out vec3 v_normal;
out float v_angle;

float noise(vec2 point) {
	return texture(permTexture, point).r;
}

mat4 translationMatrix(vec3 axis) {
	return mat4(1.0, 0.0, 0.0, 0.0,
	            0.0, 1.0, 0.0, 0.0,
	            0.0, 0.0, 1.0, 0.0,
	            axis.x, axis.y, axis.z, 1.0);
}

mat4 rotationMatrix(vec3 axis, float angle) {
	axis = normalize(axis);
	float sine = sin(angle);
	float cosine = cos(angle);
	float oneMinusCosine = 1.0 - cosine;
	return mat4(
		oneMinusCosine * axis.x * axis.x + cosine,
		oneMinusCosine * axis.x * axis.y - axis.z * sine,
		oneMinusCosine * axis.z * axis.x + axis.y * sine,
		0.0,
		oneMinusCosine * axis.x * axis.y + axis.z * sine,
		oneMinusCosine * axis.y * axis.y + cosine,
		oneMinusCosine * axis.y * axis.z - axis.x * sine,
		0.0,
		oneMinusCosine * axis.z * axis.x - axis.y * sine,
		oneMinusCosine * axis.y * axis.z + axis.x * sine,
		oneMinusCosine * axis.z * axis.z + cosine,
		0.0,
		0.0, 0.0, 0.0, 1.0);
}

void main(void) {
	vec2 position = (a_center.xy + 1.0) / 2.0;
	// A 0..1 pseudo-random value that spreads the start times of neighboring tiles.
	float fuzz = noise(position);
	float startTime = position.x * 0.5 + fuzz * 0.25;
	float endTime = startTime + 0.25;
	float transitionTime = clamp((time - startTime) / (endTime - startTime), 0.0, 1.0);
	// The tile skips the half turn where it shows its back, so it stays black for
	// as short a time as possible.
	if (transitionTime < 0.5)
		transitionTime = transitionTime / 2.0;
	else
		transitionTime = (transitionTime / 2.0) + 0.5;
	v_angle = transitionTime * M_PI * 2.0;

	mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
	mat4 transformMatrix = translationMatrix(a_center) * rotationMatrix(vec3(0.0, 1.0, 0.0), v_angle) * translationMatrix(-a_center);
	mat3 normalMatrix = mat3(transpose(inverse(transformMatrix)));
	gl_Position = u_projectionMatrix * modelViewMatrix * transformMatrix * vec4(a_position, 1.0);
	v_texturePosition = vec2((a_position.x + 1.0) / 2.0, (1.0 - a_position.y) / 2.0);
	v_normal = normalize(normalMatrix * a_normal);
}
