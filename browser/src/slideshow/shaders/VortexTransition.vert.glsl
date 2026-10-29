#version 300 es
precision highp float;

#define M_PI 3.1415926535897932384626433832795

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texCoord;
in vec2 a_tileIndex;

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_sceneTransformMatrix;
uniform mat4 u_primitiveTransformMatrix;
uniform mat4 u_operationsTransformMatrix;
uniform float time;
uniform vec2 tileCount;
uniform sampler2D permTexture;
uniform float slide;

out vec2 v_texturePosition;
out vec3 v_normal;

float noise(vec2 point) {
	return texture(permTexture, point).r;
}

mat4 translationMatrix(vec3 axis) {
	mat4 matrix = mat4(1.0);
	matrix[3] = vec4(axis, 1.0);
	return matrix;
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
	// A vertex placed here lands behind the viewer and is clipped away.
	const vec4 invalidPosition = vec4(-256.0, -256.0, -256.0, -256.0);

	// Each tile moves during only part of the transition. The leftmost tiles
	// start at once and arrive around time 0.5, when the rightmost ones start.
	// The exact window of a tile is fuzzed a bit for a more random look.
	vec2 tile = a_tileIndex / (tileCount - 1.0);
	float startTimeFuzz = noise(tile);
	// A value in -1.5..1.5, different for neighboring tiles, that places the
	// rotation axis. The extra 0.5 on each side lets some tiles turn outside the
	// slide.
	float rotationFuzz = noise(vec2((tileCount.x + a_tileIndex.x) / (tileCount.x - 1.0), tile.y)) * 3.0 - 1.5;
	float startTime = tile.x * 0.2 + startTimeFuzz * 0.2;
	float endTime = min(startTime + 0.7, 1.0);

	bool isLeavingSlide = slide < 0.5;
	float slideTime;
	if (!isLeavingSlide) {
		if (time < max(0.3, startTime)) {
			gl_Position = invalidPosition;
			return;
		}
		slideTime = 1.0 - time;
	} else {
		if (time > endTime) {
			gl_Position = invalidPosition;
			return;
		}
		slideTime = time;
	}

	bool isTurning = slideTime > startTime && slideTime <= endTime;
	bool isWall = a_normal.z < 0.5;
	if (isWall && !isTurning) {
		gl_Position = invalidPosition;
		return;
	}

	mat4 transform = mat4(1.0);
	if (isTurning) {
		float rotation = (slideTime - startTime) / (endTime - startTime);
		if (isLeavingSlide)
			rotation *= -1.0;
		if (rotation < -0.5 || rotation > 0.5) {
			gl_Position = invalidPosition;
			return;
		}
		vec3 translationVector = vec3(rotationFuzz, 0.0, 0.0);
		transform = translationMatrix(-translationVector) * rotationMatrix(vec3(0.0, 1.0, 0.0), clamp(rotation, -1.0, 1.0) * M_PI) * translationMatrix(translationVector);
	}

	mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
	mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));
	vec4 position = transform * vec4(a_position, 1.0);
	gl_Position = u_projectionMatrix * modelViewMatrix * position;
	v_normal = normalize(normalMatrix * mat3(transform) * vec3(0.0, 0.0, 1.0));
	v_normal.z *= a_normal.z;
	v_texturePosition = a_texCoord;
}
