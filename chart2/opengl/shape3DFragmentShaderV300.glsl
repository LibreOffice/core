/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 150 core

#define MAX_LIGHT_NUM 8

in vec3 positionWorldspace;
in vec3 normalCameraspace;

uniform mat4 V;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform vec4 materialColor;
uniform int twoSidesLighting;
uniform float materialShininess;
uniform vec4 lightColor[MAX_LIGHT_NUM];
uniform vec4 lightPosWorldspace[MAX_LIGHT_NUM];
uniform float lightPower[MAX_LIGHT_NUM];
uniform int lightNum;
uniform vec4 lightAmbient;
uniform int undraw;
uniform float minCoordX;

void main()
{
    if ((positionWorldspace.x <= minCoordX) && (undraw == 1))
        discard;
    vec3 colorTotal = vec3(0.0f, 0.0f, 0.0f);

    vec3 vertexPositionCameraspace = (V * vec4(positionWorldspace,1)).xyz;

    vec3 MaterialDiffuseColor = materialColor.rgb;

    vec3 normalDirectionCameraspace = normalCameraspace;
    vec3 eyeDirectionCameraspace = normalize(vec3(0, 0, 0) - vertexPositionCameraspace);
    float attenuation = 1.0;
    int i = 0;
    vec3 lightDirectionCameraspace;
    vec3 vertexToLightSource;

    vec3 totalAmbient = lightAmbient.rgb *
                        MaterialDiffuseColor *
                        materialAmbient.rgb;

    if ((twoSidesLighting == 1) && (!gl_FrontFacing))
    {
        normalDirectionCameraspace = -normalDirectionCameraspace;
    }
    for (i = 0; i < lightNum; i++)
    {
        float  LightPower = lightPower[i];
        lightDirectionCameraspace = normalize((V * lightPosWorldspace[i]).xyz);

        float cosTheta = clamp(dot(normalDirectionCameraspace,lightDirectionCameraspace), 0,1);
        vec3 lightDiffuse = LightPower *
                            attenuation *
                            lightColor[i].rgb *
                            MaterialDiffuseColor *
                            materialDiffuse.rgb *
                            cosTheta;

        vec3 specularReflection;
        if (dot(normalDirectionCameraspace, lightDirectionCameraspace) < 0)
        {
            specularReflection = vec3(0.0, 0.0, 0.0);
        }
        else
        {
            vec3 R = reflect(-lightDirectionCameraspace,normalDirectionCameraspace);
            float cosAlpha = clamp(dot(eyeDirectionCameraspace, R), 0,1);
            specularReflection = attenuation *
                                 LightPower *
                                 lightColor[i].rgb *
                                 materialSpecular.rgb *
                                 MaterialDiffuseColor *
                                 pow(max(0.0, cosAlpha), materialShininess);
        }
        colorTotal += lightDiffuse + specularReflection;

    }
    colorTotal += totalAmbient;
    gl_FragColor = vec4(colorTotal, 1.0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
