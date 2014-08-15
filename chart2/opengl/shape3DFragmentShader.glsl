/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 330 core
#define MAX_LIGHT_NUM 8

in vec3 positionWorldspace;
in vec3 normalCameraspace;
uniform mat4 V;
out vec4 actualColor;
struct MaterialParameters
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 materialColor;

    int twoSidesLighting;
    float shininess;
    float pad;
    float pad1;
};

layout(std140) uniform GlobalMaterialParameters
{
    MaterialParameters matralParameter;
}Material;

struct LightSource
{
    vec4   lightColor;
    vec4   positionWorldspace;
    float  lightPower;
    float  pad1;
    float  pad2;
    float  pad3;
};

layout(std140) uniform GlobalLights
{
    int         lightNum;
    vec4        ambient;
    LightSource light[MAX_LIGHT_NUM];
} Lights;

void main()
{
    vec3 colorTotal = vec3(0.0f, 0.0f, 0.0f);

    vec3 vertexPositionCameraspace = (V * vec4(positionWorldspace,1)).xyz;

    vec3 MaterialDiffuseColor = Material.matralParameter.materialColor.rgb;

    vec3 normalDirectionCameraspace = normalCameraspace;
    vec3 eyeDirectionCameraspace = normalize(vec3(0, 0, 0) - vertexPositionCameraspace);
    float attenuation = 1.0;
    int i = 0;
    vec3 lightDirectionCameraspace;
    vec3 vertexToLightSource;

    vec3 lightAmbient = Lights.ambient.rgb *
                        MaterialDiffuseColor *
                        Material.matralParameter.ambient.rgb
                        * 5.0;

    if ((Material.matralParameter.twoSidesLighting == 1) && (!gl_FrontFacing))
    {
        normalDirectionCameraspace = -normalDirectionCameraspace;
    }
    for (i = 0; i < Lights.lightNum; i++)
    {
        float  LightPower = Lights.light[i].lightPower;
        lightDirectionCameraspace = normalize((V * Lights.light[i].positionWorldspace).xyz);

        float cosTheta = clamp(dot(normalDirectionCameraspace,lightDirectionCameraspace), 0,1);
        vec3 lightDiffuse = LightPower *
                            attenuation *
                            Lights.light[i].lightColor.rgb *
                            MaterialDiffuseColor *
                            Material.matralParameter.diffuse.rgb *
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
                                 Lights.light[i].lightColor.rgb *
                                 Material.matralParameter.specular.rgb *
                                 MaterialDiffuseColor *
                                 pow(max(0.0, cosAlpha), Material.matralParameter.shininess);
        }
        colorTotal += lightDiffuse + specularReflection;

    }
    colorTotal += lightAmbient;
    actualColor = vec4(colorTotal, 1.0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
