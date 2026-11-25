#version 300 es
precision mediump float;

uniform sampler2D slideTexture;
uniform bool u_lit;

in vec2 v_texturePosition;
in vec3 v_normal;

out vec4 outColor;

void main() {
	vec4 fragment = texture(slideTexture, v_texturePosition);
	if (u_lit) {
		const float ambient = 0.55;
		vec3 lightVector = normalize(vec3(-0.3, 0.5, 1.0));
		float light = ambient + (1.0 - ambient) * max(dot(lightVector, normalize(v_normal)), 0.0);
		fragment.rgb *= light;
	}
	outColor = vec4(fragment.rgb, 1.0);
}
