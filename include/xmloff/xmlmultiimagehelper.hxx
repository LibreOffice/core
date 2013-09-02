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

#ifndef _XMLOFF_XMLMULTIIMAGEHELPER_HXX
#define _XMLOFF_XMLMULTIIMAGEHELPER_HXX

#include <sal/types.h>
#include <xmloff/xmlimp.hxx>

//////////////////////////////////////////////////////////////////////////////

class MultiImageImportHelper
{
private:
    std::vector< SvXMLImportContextRef* >       maImplContextVector;
    bool                                        mbSupportsMultipleContents;

protected:
    /// helper to get the created xShape instance, needs to be overloaded
    virtual OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const = 0;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const = 0;

public:
    MultiImageImportHelper();
    virtual ~MultiImageImportHelper();

    /// solve multiple imported images. The most valuable one is choosen,
    /// see imlementation for evtl. changing weights and/or adding filetypes.
    ///
    /// @returns import context of the selected image
    SvXMLImportContextRef solveMultipleImages();

    /// add a content to the remembered image import contexts
    void addContent(const SvXMLImportContext& rSvXMLImportContext);

    /// read/write access to boolean switch
    bool getSupportsMultipleContents() const { return mbSupportsMultipleContents; }
    void setSupportsMultipleContents(bool bNew) { mbSupportsMultipleContents = bNew; }
};

//////////////////////////////////////////////////////////////////////////////

#endif  //  _XMLOFF_XMLMULTIIMAGEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
