/*************************************************************************
 *
 *  $RCSfile: MasterPageContainer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:59:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_HXX
#define SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_HXX

#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <vcl/image.hxx>
#include <memory>
#include "PreviewRenderer.hxx"
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#include "tools/SdGlobalResourceContainer.hxx"

class SdPage;
class SdDrawDocument;
class SfxObjectShellLock;

namespace sd {
class DrawDocShell;
}

namespace sd { namespace toolpanel { namespace controls {


/** This singleton container manages the master pages of the master page
    selector controls of the tool panel.  It maintains its own document to
    store master page objects.  For each master page container stores its
    URL, preview bitmap, page name, and, if available, the page object.

    The lifetime is limited by the SdGlobalResourceContainer to that of the
    sd module.
*/
class MasterPageContainer
    : public SdGlobalResource
{
public:
    typedef int Token;
    static const Token NIL_TOKEN = -1;

    static MasterPageContainer& Instance (void);

    /** Put the master page identified and described by the given parameters
        into the container.  When there already is a master page with the
        given URL, page name, or object pointer (when that is not NULL) then
        the existing entry is replaced/updated by the given one.  Otherwise
        a new entry is inserted.
    */
    Token PutMasterPage (
        const String& sURL,
        const String& sPageName,
        SdPage* pMasterPage,
        Image aPreview);

    int GetTokenCount (void);
    /** Return a token for an index in the range
        0 <= index < GetTokenCount().
    */
    Token GetTokenForIndex (int nIndex);

    Token GetTokenForURL (const String& sURL);
    Token GetTokenForPageName (const String& sPageName);
    Token GetTokenForPageObject (const SdPage* pPage);

    String GetURLForToken (Token aToken);
    String GetPageNameForToken (Token aToken);
    SdPage* GetSlideForToken (Token aToken, bool bLoad=true);
    SdPage* GetPageObjectForToken (Token aToken, bool bLoad=true);

    /** This version of GetPreviewForToken() does not load a template when
        the preview in the requested size does not exist and the template
        has not been loaded previously.
        @param aToken
            This token specifies for which master page to return the
            prview.  The token is returned by one of the GetTokenFor...()
            functions.
        @param nWidth
            The width of the requested preview.
        @return
            The returned image is the requested preview if a) it has been
            created in the right size previously or b) the template has been
            loaded and the preview can be created.  Otherwise an empty
            substitution is returned.
    */
    Image GetPreviewForToken (
        Token aToken,
        int nWidth);

    /** This version of GetPreviewForToken() creates the requested preview
        bitmap if it does not yet exist.  As this is done asynchronously the
        caller has to supply callback funtion and user data that is passed
        to that function when the preview is available.  The preview can
        then be obtained by the two argument version of this method.
        @param aToken
            This token specifies for which master page to return the
            prview.  The token is returned by one of the GetTokenFor...()
            functions.
        @param nWidth
            The width of the requested preview.
        @param rCallback
            This callback is called when the preview is created
            asynchronously and the creation is completed.  When the preview
            is not created asynchronously, because it is already available,
            the callback is not called.
        @param pUserData
            This data is stored unaltered and passed to the callback.
    */
    Image GetPreviewForToken (
        Token aToken,
        int nWidth,
        const Link& rCallback,
        void* pUserData);

    /** The specified listener is not available anymore and must not
        be called back. All requests for preview creation for this
        callback are removed.
    */
    void RemoveCallback (const Link& rCallback);

private:
    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    MasterPageContainer (void);
    virtual ~MasterPageContainer (void);

    void LateInit (void);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
