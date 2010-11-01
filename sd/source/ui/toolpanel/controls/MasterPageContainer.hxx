/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_HXX
#define SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_HXX

#include "MasterPageContainerProviders.hxx"

#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <vcl/image.hxx>
#include <memory>
#include "PreviewRenderer.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <vcl/timer.hxx>
#include "tools/SdGlobalResourceContainer.hxx"

#include <boost/shared_ptr.hpp>

class SdPage;
class SdDrawDocument;
class SfxObjectShellLock;

namespace sd {
class DrawDocShell;
}

namespace sd { namespace toolpanel { namespace controls {

class MasterPageDescriptor;

/** This container manages the master pages used by the MasterPagesSelector
    controls.  It uses internally a singleton implementation object.
    Therefore, all MasterPageContainer object operator on the same set of
    master pages.  Each MasterPageContainer, however, has its own
    PreviewSize value and thus can independantly switch between large and
    small previews.

    The container maintains its own document to store master page objects.

    For each master page container stores its URL, preview bitmap, page
    name, and, if available, the page object.

    Entries are accessed via a Token, which is mostly a numerical index but
    whose values do not neccessarily have to be consecutive.
*/
class MasterPageContainer
{
public:
    typedef int Token;
    static const Token NIL_TOKEN = -1;

    MasterPageContainer (void);
    virtual ~MasterPageContainer (void);

    void AddChangeListener (const Link& rLink);
    void RemoveChangeListener (const Link& rLink);

    enum PreviewSize { SMALL, LARGE };
    /** There are two different preview sizes, a small one and a large one.
        Which one is used by the called container can be changed with this
        method.
        When the preview size is changed then all change listeners are
        notified of this.
    */
    void SetPreviewSize (PreviewSize eSize);

    /** Returns the preview size.
    */
    PreviewSize GetPreviewSize (void) const;

    /** Return the preview size in pixels.
    */
    Size GetPreviewSizePixel (void) const;

    enum PreviewState { PS_AVAILABLE, PS_CREATABLE, PS_PREPARING, PS_NOT_AVAILABLE };
    PreviewState GetPreviewState (Token aToken);

    /** This method is typically called for entries in the container for
        which GetPreviewState() returns OS_CREATABLE.  The creation of the
        preview is then scheduled to be executed asynchronously at a later
        point in time.  When the preview is available the change listeners
        will be notified.
    */
    bool RequestPreview (Token aToken);

    /** Each entry of the container is either the first page of a template
        document or is a master page of an Impress document.
    */
    enum Origin {
        MASTERPAGE,  // Master page of a document.
        TEMPLATE,    // First page of a template file.
        DEFAULT,     // Empty master page with default style.
        UNKNOWN
    };

    /** Put the master page identified and described by the given parameters
        into the container.  When there already is a master page with the
        given URL, page name, or object pointer (when that is not NULL) then
        the existing entry is replaced/updated by the given one.  Otherwise
        a new entry is inserted.
    */
    Token PutMasterPage (const ::boost::shared_ptr<MasterPageDescriptor>& rDescriptor);
    void AcquireToken (Token aToken);
    void ReleaseToken (Token aToken);

    /** This and the GetTokenForIndex() methods can be used to iterate over
        all members of the container.
    */
    int GetTokenCount (void) const;

    /** Determine whether the container has a member for the given token.
    */
    bool HasToken (Token aToken) const;

    /** Return a token for an index in the range
        0 <= index < GetTokenCount().
    */
    Token GetTokenForIndex (int nIndex);

    Token GetTokenForURL (const String& sURL);
    Token GetTokenForStyleName (const String& sStyleName);
    Token GetTokenForPageObject (const SdPage* pPage);

    String GetURLForToken (Token aToken);
    String GetPageNameForToken (Token aToken);
    String GetStyleNameForToken (Token aToken);
    SdPage* GetPageObjectForToken (Token aToken, bool bLoad=true);
    Origin GetOriginForToken (Token aToken);
    sal_Int32 GetTemplateIndexForToken (Token aToken);
    ::boost::shared_ptr<MasterPageDescriptor> GetDescriptorForToken (Token aToken);

    void InvalidatePreview (Token aToken);

    /** Return a preview for the specified token.  When the preview is not
        present then the PreviewProvider associated with the token is
        executed only when that is not expensive.  It is the responsibility
        of the caller to call RequestPreview() to do the same
        (asynchronously) for expensive PreviewProviders.
        Call GetPreviewState() to find out if that is necessary.
        @param aToken
            This token specifies for which master page to return the prview.
            Tokens are returned for example by the GetTokenFor...() methods.
        @return
            The returned image is the requested preview or a substitution.
    */
    Image GetPreviewForToken (Token aToken);

private:
    class Implementation;
    ::boost::shared_ptr<Implementation> mpImpl;
    PreviewSize mePreviewSize;

    /** Retrieve the preview of the document specified by the given URL.
    */
    static BitmapEx LoadPreviewFromURL (const ::rtl::OUString& aURL);
};




/** For some changes to the set of master pages in a MasterPageContainer or
    to the data stored for each master page one or more events are sent to
    registered listeners.
    Each event has an event type and a token that tells the listener where
    the change took place.
*/
class MasterPageContainerChangeEvent
{
public:
    enum EventType {
        // A master page was added to the container.
        CHILD_ADDED,
        // A master page was removed from the container.
        CHILD_REMOVED,
        // The preview of a master page has changed.
        PREVIEW_CHANGED,
        // The size of a preview has changed.
        SIZE_CHANGED,
        // Some of the data stored for a master page has changed.
        DATA_CHANGED,
        // The TemplateIndex of a master page has changed.
        INDEX_CHANGED,
        // More than one entries changed their TemplateIndex
        INDEXES_CHANGED
    } meEventType;

    // Token of the container entry whose data changed or which was added or
    // removed.
    MasterPageContainer::Token maChildToken;
};


} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
