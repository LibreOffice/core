#version 300 es
precision mediump float;

uniform sampler2D slideTexture;
uniform float selectedTexture;
uniform float time;
uniform float hexagonSize;

in vec2 v_texturePosition;
in float v_fuzz;
in vec2 v_center;
in vec3 v_normal;

out vec4 outColor;

bool isBorder(vec2 point) {
	return point.x < 0.02 || point.x > 0.98 || point.y < 0.02 || point.y > 0.98;
}

void main() {
	vec4 fragment = vec4(texture(slideTexture, v_texturePosition).rgb, 1.0);
	vec3 lightVector = vec3(0.0, 0.0, 1.0);
	float light = max(dot(lightVector, v_normal), 0.0);

	if (hexagonSize > 1.0) {
		// The gap filler fades as one sheet, with a slight left to right lag, so the
		// gaps open before the cells go.
		if (selectedTexture > 0.5)
			fragment.a = 1.0 - time * 8.0 + gl_FragCoord.x / 1024.0;
		else
			fragment.a = time * 8.0 - 7.3 + gl_FragCoord.x / 1024.0;
	} else {
		// The cells themselves, each one on its own schedule. A cell whose center
		// is outside the slide goes first on the way out and last on the way in.
		float startTime;
		float actualTime;
		if (selectedTexture > 0.5) {
			if (isBorder(v_center))
				startTime = 0.15;
			else
				startTime = 0.15 + v_fuzz * 0.4;
			float endTime = startTime + 0.05;
			actualTime = 1.0 - clamp((time - startTime) / (endTime - startTime), 0.0, 1.0);
		} else {
			if (isBorder(v_center))
				startTime = 0.85;
			else
				startTime = 0.3 + v_fuzz * 0.4;
			float endTime = startTime + 0.05;
			actualTime = clamp((time - startTime) / (endTime - startTime), 0.0, 1.0);
			if (time < 0.8)
				actualTime *= time / 0.8;
		}
		if (selectedTexture > 0.5) {
			// The leaving cells go transparent, so the entering slide shows through.
			fragment.a = actualTime;
		} else {
			// The entering cells light up from black.
			fragment.rgb *= actualTime;
		}
	}

	vec4 black = vec4(0.0, 0.0, 0.0, fragment.a);
	outColor = mix(black, fragment, light);
}
