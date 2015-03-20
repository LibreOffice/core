/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SVPFORLOKIT_HXX
#define INCLUDED_VCL_SVPFORLOKIT_HXX

#include <vcl/virdev.hxx>
#include <vcl/dllapi.h>

// These functions are for use by LibreOfficeKit only, I think

class SalFrame;

VCL_DLLPUBLIC void InitSvpForLibreOfficeKit();
VCL_DLLPUBLIC int GetRowStrideForLibreOfficeKit(SalVirtualDevice* pVD);
VCL_DLLPUBLIC SalFrame* GetSvpFocusFrameForLibreOfficeKit();
VCL_DLLPUBLIC vcl::Window* GetSalFrameWindowForLibreOfficeKit(SalFrame *pSF);

#endif // INCLUDED_VCL_SVPFORLOKIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
