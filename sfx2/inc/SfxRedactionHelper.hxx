/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_INC_SFXREDACTIONHELPER_HXX
#define INCLUDED_CUI_SOURCE_INC_SFXREDACTIONHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XComponent.hpp>

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

class SfxRequest;
class SfxStringItem;
class GDIMetaFile;
class DocumentToGraphicRenderer;
class SfxViewFrame;

/*
 * Mostly a bunch of static methods to handle the redaction functionality at
 * different points of the process.
 **/
class SfxRedactionHelper
{
public:
    /// Checks to see if the request has a parameter of IsRedactMode:bool=true
    static bool isRedactMode(const SfxRequest& rReq);
    /*
     * Returns the value of the given string param as an OUString
     * Returns empty OUString if no param
     * */
    static OUString getStringParam(const SfxRequest& rReq, const sal_uInt16& nParamId);
    /*
     * Creates metafiles from the pages of the given document,
     * and pushes into the given vector.
     * */
    static void getPageMetaFilesFromDoc(std::vector<GDIMetaFile>& aMetaFiles,
                                        const sal_Int32& nPages,
                                        DocumentToGraphicRenderer& aRenderer, bool bIsWriter,
                                        bool bIsCalc);
    /*
     * Creates one shape and one draw page for each gdimetafile,
     * and inserts the shapes into the newly created draw pages.
     * */
    static void addPagesToDraw(uno::Reference<XComponent>& xComponent, const sal_Int32& nPages,
                               const std::vector<GDIMetaFile>& aMetaFiles, bool bIsCalc);
    /*
     * Makes the Redaction toolbar visible to the user.
     * Meant to be called after converting a document to a Draw doc
     * for redaction purposes.
     * */
    static void showRedactionToolbar(SfxViewFrame* pViewFrame);
};

#endif // INCLUDED_CUI_SOURCE_INC_SFXREDACTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
