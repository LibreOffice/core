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
#pragma once

#include <loadenv/actionlockguard.hxx>

#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/URL.hpp>
#include <rtl/ref.hxx>
#include <unotools/mediadescriptor.hxx>
#include <o3tl/typed_flags_set.hxx>


/** @short  enable/disable special features
            of a load request.

    @desrc  Such features must outcome without
            any special parameters.
            To make enabling/disabling of
            features very easy (e.g. at the ctor of
             this class) these values must be combinable
            as flags. That means: its values must be in
            range of [2^n]!
 */
enum class LoadEnvFeatures
{
    /// we should be informed, if no feature is enabled :-)
    NONE = 0,
    /// enable using of UI elements during loading (means progress, interaction handler etcpp.)
    WorkWithUI = 1,
    /// enable loading of resources, which are not related to a target frame! (see concept of ContentHandler)
    AllowContentHandler = 2
};
namespace o3tl {
    template<> struct typed_flags<LoadEnvFeatures> : is_typed_flags<LoadEnvFeatures, 0x3> {};
}


namespace framework {

class QuietInteraction;

/** @short  implements general mechanism for loading documents.

    @descr  An instance of this class can be used inside the API calls
            XComponentLoader::loadComponentFromURL() and
            XDispatch::dispatch().
 */
class LoadEnv
{
public:
    /** @short  classify a content.

        @descr  The load environment must know, if a content
                is related to a target frame or not. Only "visible"
                components, which fulfill the requirements of the
                model-controller-view paradigm can be loaded into a frame.
                Such contents are classified as E_CAN_BE_LOADED.

                But e.g. for the dispatch framework exists special ContentHandler
                objects, which can load a content in "non visible" mode ...
                and do not need a target frame for its operation. Such
                ContentHandler e.g. plays sounds.
                Such contents are classified as E_CAN_BE_HANDLED.

                And last but not least a content can be "not valid" in general.
     */
    enum EContentType
    {
        /// identifies a content, which seems to be invalid in general
        E_UNSUPPORTED_CONTENT,
        /// identifies a content, which can be used with a ContentHandler and is not related to a target frame
        E_CAN_BE_HANDLED,
        /// identifies a content, which can be loaded into a target frame
        E_CAN_BE_LOADED,
        /// special mode for non real loading, In such case the model is given directly!
        E_CAN_BE_SET
    };

private:
    mutable osl::Mutex m_mutex;

    /** @short  reference to a uno service manager, which must be used
                to created on needed services on demand.
     */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /** @short  points to the frame, which uses this LoadEnv object
                and must be used to start target search there.
     */
    css::uno::Reference< css::frame::XFrame > m_xBaseFrame;

    /** @short  points to the frame, into which the new component was loaded.

        @descr  Note: This reference will be empty if loading failed
                or a non visible content was loaded!
                It can be the same frame as m_xBaseFrame it describe, in case
                the target "_self", "" or the search flag "SELF" was used.
                Otherwise it's the new created or recycled frame, which was
                used for loading and contains further the new component.

                Please use method getTarget() or getTargetComponent()
                to return the frame/controller or model to any interested
                user of the results of this load request.
     */
    css::uno::Reference< css::frame::XFrame > m_xTargetFrame;

    /** @short  contains the name of the target, in which the specified resource
                of this instance must be loaded.
     */
    OUString m_sTarget;

    /** @short  if m_sTarget is not a special one, this flags regulate searching
                of a suitable one.
     */
    sal_Int32 m_nSearchFlags;

    /** @short  contains all needed information about the resource,
                which should be loaded.

        @descr  Inside this struct e.g. the URL, its type and filter name,
                the stream or a model directly are saved.
     */
    utl::MediaDescriptor m_lMediaDescriptor;

    /** @short  because the mediadescriptor contains the complete URL ... but
                some functionality need the structured version, we hold it twice :-(.
     */
    css::util::URL m_aURL;

    /** @short  enable/disable special features of a load request. */
    LoadEnvFeatures m_eFeature;

    /** @short  classify the content, which should be loaded by this instance. */
    EContentType m_eContentType;

    /** @short  it indicates, that the member m_xTargetFrame was new created for this
                load request and must be closed in case loading (not handling!)
                operation failed. The default value is sal_False!
     */
    bool m_bCloseFrameOnError;

    /** @short  it indicates, that the old document (which was located inside m_xBaseFrame
                in combination with the m_sTarget value "_self") was suspended.
                Normally it will be replaced by the new loaded document. But in case
                loading (not handling!) failed, it must be reactivated.
                The default value is sal_False!
     */
    bool m_bReactivateControllerOnError;

    /** @short  it holds one (!) asynchronous used contenthandler or frameloader
                alive, till the asynchronous operation will be finished.
     */
    css::uno::Reference< css::uno::XInterface > m_xAsynchronousJob;

    /** @short  holds the information about the finished load process.

        @descr  The content of m_xTargetFrame can't be used as valid indicator,
                (in case the existing old document was reactivated)
                we must hold the result of the load process explicitly.
     */
    bool m_bLoaded;

    /** @short  If we already brought it to front; do not do that again
                (the user could switch elsewhere after the first activation,
                and we shouldn't nag them again).
     */
    bool m_bFocusedAndToFront = false;

    /** @short      holds an XActionLock on the internal used task member.

        @seealso    m_xTargetFrame
     */
    ActionLockGuard m_aTargetLock;

    rtl::Reference<QuietInteraction> m_pQuietInteraction;

public:

    /** @short  initialize a new instance of this load environment.

        @param  xContext
                reference to a uno service manager, which can be used internally
                to create on needed services on demand.

        @throw  Currently there is no reason to throw such exception!

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    LoadEnv(css::uno::Reference< css::uno::XComponentContext >  xContext);

    /** @short  deinitialize an instance of this class in the right way.
     */
    ~LoadEnv();

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    static css::uno::Reference< css::lang::XComponent > loadComponentFromURL(const css::uno::Reference< css::frame::XComponentLoader >&    xLoader,
                                                                             const css::uno::Reference< css::uno::XComponentContext >&     xContext,
                                                                             const OUString&                                        sURL   ,
                                                                             const OUString&                                        sTarget,
                                                                                   sal_Int32                                               nFlags ,
                                                                             const css::uno::Sequence< css::beans::PropertyValue >&        lArgs  );

    /** @short  start loading of a resource

        @descr  The parameter for targeting, the content description, and
                some environment specifier (UI, dispatch functionality)
                can be set here. Of course a still running load request
                will be detected here and a suitable exception will be thrown.
                Such constellation can be detected outside by using provided
                synchronisation methods or callbacks.

                There is no direct return value possible here. Because it depends
                from the usage of this instance! E.g. for loading a "visible component"
                a frame with a controller/model inside can be possible. For loading
                of a "non visible component" only an information about a successfully start
                can be provided.
                Further it can't be guaranteed, that the internal process runs synchronous.
                that's why we prefer using of specialized methods afterwards e.g. to:
                    - wait till the internal job will be finished
                      and get the results
                    - or to let it run without any further control from outside.

        @param  sURL
                points to the resource, which should be loaded.

        @param  lMediaDescriptor
                contains additional information for the following load request.

        @param  xBaseFrame
                points to the frame which must be used as start point for target search.

        @param  sTarget
                regulate searching/creating of frames, which should contain the
                new loaded component afterwards.

        @param  nSearchFlags
                regulate searching of targets, if sTarget is not a special one.

        @param  eFeature
                flag field, which enable/disable special features of this
                new instance for following load call.

        @throw  A LoadEnvException e.g. if another load operation is till in progress
                or initialization of a new one fail by other reasons.
                The real reason, a suitable message and ID will be given here immediately.

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    void startLoading(const OUString&                                           sURL            ,
                           const css::uno::Sequence< css::beans::PropertyValue >&    lMediaDescriptor,
                           const css::uno::Reference< css::frame::XFrame >&          xBaseFrame      ,
                           const OUString&                                           sTarget         ,
                                 sal_Int32                                           nSearchFlags    ,
                                 LoadEnvFeatures                                     eFeature        = LoadEnvFeatures::NONE);

    /** @short  wait for an already running load request (started by calling
                startLoading() before).

        @descr  The timeout parameter can be used to wait some times only
                or forever. The return value indicates if the load request
                was finished during the specified timeout period.
                But it indicates not, if the load request was successful or not!

        @param  nTimeout
                specify a timeout in [ms].
                A value 0 let it wait forever!

        @return sal_True if the started load process could be finished in time;
                sal_False if the specified time was over.

        @throw  ... currently not used :-)

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    bool waitWhileLoading(sal_uInt32 nTimeout = 0);

    /** TODO document me ... */
    css::uno::Reference< css::lang::XComponent > getTargetComponent() const;

public:

    /** @short      checks if the specified content can be handled by a
                    ContentHandler only and is not related to a target frame,
                    or if it can be loaded by a FrameLoader into a target frame
                    as "visible" component.

        @descr      using:
                        switch(classifyContent(...))
                        {
                            case E_CAN_BE_HANDLED :
                                handleIt(...);
                                break;

                            case E_CAN_BE_LOADED :
                                xFrame = locateTargetFrame();
                                loadIt(xFrame);
                                break;

                            case E_NOT_A_CONTENT :
                            default              : throw ...;
                        }

        @param      sURL
                    describe the content.

        @param      lMediaDescriptor
                    describe the content more detailed!

        @return     A suitable enum value, which classify the specified content.
     */
    static EContentType classifyContent(const OUString&                                 sURL            ,
                                        const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor);

    /** TODO document me ... */
    static  void initializeUIDefaults(
                const css::uno::Reference< css::uno::XComponentContext >& i_rxContext,
                utl::MediaDescriptor& io_lMediaDescriptor,
                const bool _bUIMode,
                rtl::Reference<QuietInteraction>* o_ppQuiteInteraction
            );

    /** TODO document me ... */
    void impl_setResult(bool bResult);

    /** TODO document me ... */
    css::uno::Reference< css::uno::XInterface > impl_searchLoader();

    /** @short  it means; show the frame, bring it to front,
                might set the right icon etcpp. in case loading was
                successfully or reactivate a might existing old document or
                close the frame if it was created before in case loading failed.

        @throw  A LoadEnvException only in cases, where an internal error indicates,
                that the complete load environment seems to be not usable in general.
                In such cases a RuntimeException would be to hard for the outside code :-)

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    void impl_reactForLoadingState();

private:
    void start();

    /** @short  tries to detect the type and the filter of the specified content.

        @descr  This method update the available media descriptor of this instance,
                so it contains the right type, a corresponding filter, may a
                valid frame loader etc. In case detection failed, this descriptor
                is corrected first, before a suitable exception will be thrown.
                (Excepting a RuntimeException occurrence!)

        @attention  Not all types we know, are supported by filters. So it does not
                    indicates an error, if no suitable filter(loader etcpp will be found
                    for a type. But a type must be detected for the specified content.
                    Otherwise it's an error and loading can't be finished successfully.

        @throw  A LoadEnvException if detection failed.

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    void impl_detectTypeAndFilter();

    /** @short  tries to use ContentHandler objects for loading.

        @descr  It searches for a suitable content handler object, registered
                for the detected content type (must be done before by calling
                impl_detectTypeAndFilter()). Because such handler does not depend
                from a real target frame, location of such frame will be
                suppressed here.
                In case handle failed all new created resources will be
                removed before a suitable exception is thrown.
                (Excepting a RuntimeException occurrence!)

        @return TODO

        @throw  A LoadEnvException if handling failed.

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    bool impl_handleContent();

    /** @short  tries to use FrameLoader objects for loading.

        @descr  First the target frame will be located. If it could be found
                or new created a filter/frame loader will be instantiated and
                used to load the content into this frame.
                In case loading failed all new created resources will be
                removed before a suitable exception is thrown.
                (Excepting a RuntimeException occurrence!)

        @return TODO

        @throw  A LoadEnvException if loading failed.

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    bool impl_loadContent();

    /** @short  checks if the specified content is already loaded.

        @descr  It depends from the set target information, if such
                search is allowed or not! So this method checks first,
                if the target is the special one "_default".
                If not it returns with an empty result immediately!
                In case search is allowed, an existing document with the
                same URL is searched. If it could be found, the corresponding
                view will get the focus and this method return the corresponding frame.
                Optional jumpmarks will be accepted here too. So the
                view of the document will be updated to show the position
                inside the document, which is related to the jumpmark.

        @return A valid reference to the target frame, which contains the already loaded content
                and could be activated successfully. An empty reference otherwise.

        @throw  A LoadEnvException only in cases, where an internal error indicates,
                that the complete load environment seems to be not usable in general.
                In such cases a RuntimeException would be to hard for the outside code :-)

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    css::uno::Reference< css::frame::XFrame > impl_searchAlreadyLoaded();

    /** @short  search for any target frame, which seems to be usable
                for this load request.

        @descr  Because this special feature is bound to the target specifier "_default"
                its checked inside first. If it's not set => this method return an empty
                reference. Otherwise any currently existing frame will be analyzed, if
                it can be used here. The following rules exists:

                <ul>
                    <li>The frame must be empty ...</li>
                    <li>or contains an empty document of the same application module
                        which the new document will have (Note: the filter of the new content
                        must be well known here!)</li>
                    <li>and(!) this target must not be already used by any other load request.</li>
                </ul>

                If a suitable target is located it will be locked. That's why the last rule
                exists! If this method returns a valid frame reference, it was locked to be usable
                for this load request only. (Don't forget to reset this state later!)
                Concurrent LoadEnv instances can synchronize her work be using such locks :-) HOPEFULLY

        @throw  A LoadEnvException only in cases, where an internal error indicates,
                that the complete load environment seems to be not usable in general.
                In such cases a RuntimeException would be to hard for the outside code :-)

        @throw  A RuntimeException in case any internal process indicates, that
                the whole runtime can't be used any longer.
     */
    css::uno::Reference< css::frame::XFrame > impl_searchRecycleTarget();

    /** @short  because showing of a frame is needed more than once...
                it's implemented as a separate method .-)

        @descr  Note: Showing of a frame is bound to a special feature...
                a) If we recycle any existing frame, we must bring it to front.
                   Showing of such frame is not needed really... because we recycle
                   visible frames only!
                b) If the document was already shown (e.g. by our progress implementation)
                   we do nothing here. The reason behind: The document was already shown...
                   and it was already make a top window...
                   If the user activated another frame inbetween (because loading needed some time)
                   it's not allowed to disturb the user again. Then the frame must resists in the background.
                c) If the frame was not shown before... but loading of a visible document into this frame
                   was finished... we need both actions: setVisible() and toFront().

        @param  xWindow
                points to the container window of a frame.

        @param  bForceToFront
                if it's set to sal_False... showing of the window is done more intelligent.
                setVisible() is called only if the window was not shown before.
                This mode is needed by b) and c)
                If it's set to sal_True... both actions has to be done: setVisible(), toFront()!
                This mode is needed by a)
     */
    static void impl_makeFrameWindowVisible(const css::uno::Reference< css::awt::XWindow >& xWindow      ,
                                           bool                                  bForceToFront);

    /** @short  checks whether a frame is already used for another load request or not.

        @descr  Such frames can't be used for our "recycle feature"!

        @param  xFrame
                the frame, which should be checked.

        @return [sal_Bool]
                sal_True if this frame is already used for loading,
                sal_False otherwise.
     */
    static bool impl_isFrameAlreadyUsedForLoading(const css::uno::Reference< css::frame::XFrame >& xFrame);

    /** @short  try to determine the used application module
                of this load request and apply right position and size
                for this document window... hopefully before we show it .-)
     */
    void impl_applyPersistentWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow);

    /** @short  determine if it's allowed to open new document frames.
     */
    bool impl_furtherDocsAllowed();

    /** @short  jumps to the requested bookmark inside a given document.
     */
    void impl_jumpToMark(const css::uno::Reference< css::frame::XFrame >& xFrame,
                         const css::util::URL&                            aURL  );

    /** @short  determine if this loader has an interactive dialog shown before
                loading the document.
     */
    bool impl_filterHasInteractiveDialog() const;

    /** @short  checks if this should bring to front and get focus on load,
                according to user settings and to the load flags.
     */
    bool shouldFocusAndToFront() const;
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
