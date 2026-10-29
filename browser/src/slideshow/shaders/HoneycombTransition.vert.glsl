#version 300 es
precision highp float;

#define M_PI 3.1415926535897932384626433832795

in vec3 a_center;
in vec2 a_offset;
in vec3 a_normal;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_sceneTransformMatrix;
uniform mat4 u_primitiveTransformMatrix;
uniform mat4 u_operationsTransformMatrix;
uniform float time;
uniform float selectedTexture;
uniform float hexagonSize;
uniform float slideRatio;
uniform sampler2D permTexture;

out vec2 v_texturePosition;
out float v_fuzz;
out vec2 v_center;
out vec3 v_normal;

const float expandFactor = 0.0318;

float noise(vec2 point) {
	return texture(permTexture, point).r;
}

mat4 translationMatrix(vec3 axis) {
	return mat4(1.0, 0.0, 0.0, 0.0,
	            0.0, 1.0, 0.0, 0.0,
	            0.0, 0.0, 1.0, 0.0,
	            axis.x, axis.y, axis.z, 1.0);
}

mat4 scaleMatrix(vec3 axis) {
	return mat4(axis.x, 0.0,    0.0,    0.0,
	            0.0,    axis.y, 0.0,    0.0,
	            0.0,    0.0,    axis.z, 0.0,
	            0.0,    0.0,    0.0,    1.0);
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
	// Only the cells get walls. The gap filler is a flat hexagon.
	bool isWall = a_normal.z < 0.5;
	if (isWall && hexagonSize >= 1.0) {
		gl_Position = vec4(-256.0, -256.0, -256.0, -256.0);
		return;
	}

	// The slide turns around its center as a circle on the screen, so the turn is
	// done in a space where the slide is square.
	mat4 slideScaleMatrix = scaleMatrix(vec3(slideRatio, 1.0, 1.0));
	mat4 invertSlideScaleMatrix = scaleMatrix(vec3(1.0 / slideRatio, 1.0, 1.0));
	mat4 transformMatrix;
	if (selectedTexture > 0.5) {
		// The leaving slide grows and comes toward the viewer.
		float growth = 1.0 + pow(2.0 * time, 2.1);
		transformMatrix = translationMatrix(vec3(0.0, 0.0, 6.0 * time))
			* scaleMatrix(vec3(growth, growth, 0.0))
			* slideScaleMatrix
			* rotationMatrix(vec3(0.0, 0.0, 1.0), -pow(time, 3.0) * M_PI)
			* invertSlideScaleMatrix;
	} else {
		// The entering slide comes from far behind and turns into place.
		float turn = 0.8 * (time - 1.0);
		transformMatrix = translationMatrix(vec3(0.0, 0.0, 28.0 * (sqrt(time) - 1.0)))
			* slideScaleMatrix
			* rotationMatrix(vec3(0.0, 0.0, 1.0), turn * turn * M_PI)
			* invertSlideScaleMatrix;
	}

	mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix * transformMatrix;
	vec3 center = vec3(a_center.xy, 0.0);
	vec4 position = vec4(center + hexagonSize * expandFactor * vec3(a_offset, 0.0) + vec3(0.0, 0.0, a_center.z), 1.0);
	gl_Position = u_projectionMatrix * modelViewMatrix * position;
	v_texturePosition = vec2(position.x + 1.0, 1.0 - position.y) / 2.0;
	v_center = (1.0 + a_center.xy) / 2.0;
	v_fuzz = noise(a_center.xy);
	v_normal = a_normal;
}
