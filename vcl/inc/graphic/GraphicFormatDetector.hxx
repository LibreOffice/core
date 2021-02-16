/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_INC_GRAPHICFORMATDETECTOR_HXX
#define INCLUDED_VCL_INC_GRAPHICFORMATDETECTOR_HXX

#include <tools/stream.hxx>
#include <vector>

namespace vcl
{
/***
 * This function is has two modes:
 * - determine the file format when bTest = false
 *   returns true, success
 *   out rFormatExtension - on success: file format string
 * - verify file format when bTest = true
 *   returns false, if file type can't be verified
 *           true, if the format is verified or the format is not known
 */
VCL_DLLPUBLIC bool peekGraphicFormat(SvStream& rStream, OUString& rFormatExtension, bool bTest);

class VCL_DLLPUBLIC GraphicFormatDetector
{
public:
    SvStream& mrStream;
    OUString maExtension;

    std::vector<sal_uInt8> maFirstBytes;
    sal_uInt32 mnFirstLong;
    sal_uInt32 mnSecondLong;

    sal_uInt64 mnStreamPosition;
    sal_uInt64 mnStreamLength;

    OUString msDetectedFormat;

    GraphicFormatDetector(SvStream& rStream, OUString const& rFormatExtension);

    bool detect();

    bool checkMET();
    bool checkBMP();
    bool checkWMForEMF();
    bool checkPCX();
    bool checkTIF();
    bool checkGIF();
    bool checkPNG();
    bool checkJPG();
    bool checkSVM();
    bool checkPCD();
    bool checkPSD();
    bool checkEPS();
    bool checkDXF();
    bool checkPCT();
    bool checkPBMorPGMorPPM();
    bool checkRAS();
    bool checkXPM();
    bool checkXBM();
    bool checkSVG();
    bool checkTGA();
    bool checkMOV();
    bool checkPDF();
};
}

#endif // INCLUDED_VCL_INC_GRAPHICFORMATDETECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
