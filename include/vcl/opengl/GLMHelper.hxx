/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_GLM_GLMHELPER_HXX
#define INCLUDED_VCL_GLM_GLMHELPER_HXX

#include "glm/glm.hpp"
#include <vcl/vclopengl_dllapi.hxx>

#include <ostream>

VCLOPENGL_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const glm::mat4& rMatrix);
VCLOPENGL_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const glm::vec4& rPos);
VCLOPENGL_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const glm::vec3& rPos);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
