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
#ifndef __FILTER_CONFIG_TYPEDETECTION_HXX_
#define __FILTER_CONFIG_TYPEDETECTION_HXX_

#include "basecontainer.hxx"
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <cppuhelper/implbase1.hxx>

//_______________________________________________
// namespace

namespace filter{ namespace config {

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements the service <type scope="com.sun.star.document">TypeDetection</type>.
 */
class TypeDetection : public ::cppu::ImplInheritanceHelper1< BaseContainer                 ,
                                                             css::document::XTypeDetection >
{
//-------------------------------------------
// native interface

public:

    //---------------------------------------
    // ctor/dtor

    /** @short  standard ctor to connect this interface wrapper to
                the global filter cache instance ...

        @param  xSMGR
                reference to the uno service manager, which created this service instance.
     */
    TypeDetection(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

    //---------------------------------------

    /** @short  standard dtor.
     */
    virtual ~TypeDetection();

//-------------------------------------------
// private helper

private:

    //---------------------------------------
    /** TODO document me */
    sal_Bool impl_getPreselectionForType(const ::rtl::OUString& sPreSelType,
                                         const css::util::URL&  aParsedURL ,
                                               FlatDetection&   rFlatTypes );

    //---------------------------------------
    /** TODO document me */
    sal_Bool impl_getPreselectionForFilter(const ::rtl::OUString& sPreSelFilter,
                                           const css::util::URL&  aParsedURL   ,
                                                 FlatDetection&   rFlatTypes   );

    //---------------------------------------
    /** TODO document me */
    sal_Bool impl_getPreselectionForDocumentService(const ::rtl::OUString& sPreSelDocumentService,
                                                     const css::util::URL& aParsedURL            ,
                                                           FlatDetection&  rFlatTypes            );

    //---------------------------------------

    /** @short      check if a filter or a type was preselected inside the given
                    MediaDescriptor and validate this information.

        @descr      Only in case the preselected filter exists and its type registration
                    seems to be usefully, it would be used realy as valid type detection
                    result. This method doesnt make any deep detection here. It checks only
                    if the preselection match to the URL by an URLPattern.
                    This information has to be added to the given rFlatTypes list too.
                    The outside code can use it to supress a deep detection then in general.
                    Because pattern are defined as non detectable at all!

        @param      pDescriptor
                    provides any easy-to-use stl interface to the MediaDescriptor.
                    Note : Its content will be adapted to returned result of this method.
                    Means: The type/filter entries of it will be actualized or removed.

        @param      rFlatTypes
                    the preselected type (or the registered type of a preselected filter)
                    will be added here as first(!) element. Further we have to provide the
                    information, if this type match to the given URL by its URLPattern
                    registration.
     */
    void impl_getPreselection(const css::util::URL&                aParsedURL ,
                                    ::comphelper::MediaDescriptor& rDescriptor,
                                    FlatDetection&                 rFlatTypes );

    //---------------------------------------

    /** @short      make a combined flat/deep type detection

        @descr      It steps over all flat detected types (given by the parameter lFlatTypes),
                    try it and search for most suitable one.
                    The specified MediaDescriptor will be patched, so it contain
                    the right values everytime. Using of any deep detection service
                    can be enabled/disabled. And last but not least: If the results
                    wont be realy clear (because a flat detected type has no deep
                    detection service), a "sugested" type name will be returned as "rLastChance".
                    It can be used after e.g. all well known deep detection services
                    was used without getting any result. Then this "last-chance-type"
                    should be returned. Of course using of it can fail too ... but its a try :-)

                    As an optimization - this method collects the names of all used deep
                    detection services. This information can be usefull inside the may be
                    afterwards called method "impl_detectTypeDeepOnly()"!

        @param      rDescriptor
                    provides any easy-to-use stl interface to the MediaDescriptor.
                    Note : Its content will be adapted to returned result of this method.
                    Means: The type/filter entries of it will be actualized or removed from it.

        @param      lFlatTypes
                    a list of all flat detected types, which should be checked here.
                    No other types are allowed here!

        @param      rLastChance
                    the internal name of a "suggested type" ... (see before)
                    Note: it will be reseted to an empty string everytimes. So
                    a set value of "rLastChance" can be detected outside very easy.

        @param      rUsedDetectors
                    used as [out] parameter. It contains a list of names of all deep
                    detection services, which was used inside this method.
                    Such detectors can be ignored later if "impl_detectTypeDeepOnly()"
                    is called.

        @param      bAllowDeep
                    enable/disable using of a might existing deep detection service.

        @return     The internal name of a detected type.
                    An empty value if detection failed. .... but see rLastChance
                    for additional returns!
     */
    ::rtl::OUString impl_detectTypeFlatAndDeep(      ::comphelper::MediaDescriptor& rDescriptor   ,
                                               const FlatDetection&                 lFlatTypes    ,
                                                     sal_Bool                       bAllowDeep    ,
                                                     OUStringList&                  rUsedDetectors,
                                                     ::rtl::OUString&               rLastChance   );

    //---------------------------------------

    /** @short      make a deep type detection only

        @descr      It steps over all well known deep detection services
                    and check her results. The first positive result will be
                    used for return. Its more a "try and error" algorithm then
                    a real type detection and will be used if a flat detection
                    cant work realy ... e.g. if the extension of an URL is
                    missing or wrong.

        @param      rDescriptor
                    provides any easy-to-use stl interface to the MediaDescriptor.
                    Note : Its content will be adapted to returned result of this method.
                    Means: The type/filter entries of it will be actualized or removed from it.

        @param      rUsedDetectors
                    It contains a list of names of all deep detection services,
                    which was already used inside the method "impl_detectTypeFlatAndDeep()"!
                    Such detectors must be ignored here!

        @return     The internal name of a detected type.
                    An empty value if detection failed.
     */
    ::rtl::OUString impl_detectTypeDeepOnly(      ::comphelper::MediaDescriptor& rDescriptor   ,
                                            const OUStringList&                  rUsedDetectors);

    //---------------------------------------

    /** @short      seek a might existing stream to position 0.

        @descr      This is an optinal action to be more robust
                    in case any detect service doesnt make this seek ...
                    Normaly it's part of any called detect service or filter ...
                    but sometimes it's not done there.

        @param      rDescriptor
                    a stl representation of the MediaDescriptor as in/out parameter.
     */
    void impl_seekStreamToZero(comphelper::MediaDescriptor& rDescriptor);

    //---------------------------------------

    /** @short      make deep type detection for a specified
                    detect service (threadsafe!).

        @descr      It creates the right uno service, prepare the
                    needed MediaDescriptor, call ths right interfaces,
                    and return the results.

        @attention  The results (means type and corresponding filter)
                    are already part of the in/out parameter pDescriptor.
                    (in case they was valid).

        @param      sDetectService
                    uno service name of the detect service.

        @param      rDescriptor
                    a stl representation of the MediaDescriptor as in/out parameter.
     */
    ::rtl::OUString impl_askDetectService(const ::rtl::OUString&               sDetectService,
                                                ::comphelper::MediaDescriptor& rDescriptor   );

    //---------------------------------------

    /** @short      try to find an interaction handler and
                    ask him to select a possible filter for
                    this unknown format.

        @descr      If the user select a filter, it will be used as return value
                    without further checking against the given file content!

        @param      rDescriptor
                    a stl representation of the MediaDescriptor as in/out parameter.

        @return     [string]
                    a valid type name or an empty string if user canceled interaction.
     */
    ::rtl::OUString impl_askUserForTypeAndFilterIfAllowed(::comphelper::MediaDescriptor& rDescriptor);

    //---------------------------------------

    /** @short      check if an input stream is already part of the
                    given MediaDesciptor and creates a new one if neccessary.

        @attention  This method does further something special!
                    <ul>
                        <li>
                            If the given URL seem to be a streamable content, but creation of the stream
                            failed (might by an IOException), this method throws an exception.
                            (May be an existing interaction handler must be called here too ...)
                            The whole detection must be interrupted then and the interface method queryTypeByDescriptor()
                            must return an empty type name value.

                            That prevent us against multiple handling of the same error more then ones
                            (e.g. if we ask all detect services as fallback ...).
                        </li>
                        <li>
                            In case the stream already exists inside the descriptor this method does nothing.
                        </li>
                        <li>
                            In case the stream does not exists but can be created successfully, the stream will
                            be added to the descriptor.
                        </li>
                    </ul>

        @param      rDescriptor
                    provides any easy-to-use stl interface to the MediaDescriptor.
                    Note : Its content will be adapted to returned result of this method.
                    Means: The stream will be added to it.

        @throw      Any suitable exception if stream should be opened but operation was not sucessfull.
                    Note: If an interactionHandler is part of the given descriptor too, it was already used.
                    Means: let the exception pass trough the top most interface method!
     */
    void impl_openStream(::comphelper::MediaDescriptor& rDescriptor)
        throw (css::uno::Exception);

    //---------------------------------------

    /** @short      validate the specified type and its relation ships
                    and set all needed informations related to this type
                    in the specified descriptor.

        @descr      Related informations are: - corresponding filter
                                              - media type
                                              - ...

        @param      rDescriptor
                    provides access to the outside MediaDescriptor.

        @param      sType
                    the name of the type, which should be set on the descriptor.
                    Can be empty to remove any related value from the descriptor!

        @return     TRUE the specified type and its registrations was valid(!) and
                    could be set on the descriptor.
     */
    sal_Bool impl_validateAndSetTypeOnDescriptor(      ::comphelper::MediaDescriptor& rDescriptor,
                                                 const ::rtl::OUString&               sType      );

    //---------------------------------------

    /** @short      validate the specified filter and its relation ships
                    and set all needed informations related to this filter
                    in the specified descriptor.

        @descr      Related informations are: - corresponding type
                                              - ...

        @param      rDescriptor
                    provides access to the outside MediaDescriptor.

        @param      sFilter
                    the name of the filter, which should be set on the descriptor.
                    Can be empty to remove any related value from the descriptor!

        @return     TRUE the specified type and its registrations was valid(!) and
                    could be set on the descriptor.
     */
    sal_Bool impl_validateAndSetFilterOnDescriptor(      ::comphelper::MediaDescriptor& rDescriptor,
                                                   const ::rtl::OUString&               sFilter    );

    //---------------------------------------

    /** @short      remove anythimng related to a TYPE/FILTER entry from the
                    specified MediaDescriptor.

        @descr      This method works together with impl_validateAndSetTypeOnDescriptor()/
                    impl_validateAndSetFilterOnDescriptor(). All informations, which can be
                    set by these two operations must be "removable" by this method.

        @param      rDescriptor
                    reference to the MediaDescriptor (represented by an easy-to-use
                    stl interface!), which should be patched.
     */
    void impl_removeTypeFilterFromDescriptor(::comphelper::MediaDescriptor& rDescriptor);

    //---------------------------------------

    /** @short      search the best suitable filter for the given type
                    and add it into the media descriptor.

        @descr      Normaly this is a type detection only ...
                    but for some special features we must overwrite our detection
                    because a file must be loaded into a special (means preselected)
                    application.

                    E.g. CSV/TXT format are sometimes ugly to handle .-)

                    Note: If the descriptor already include a filter
                    (may be selected by a FilterSelect interaction or preselected
                    by the user itself) ... we dont change that here !

        @param      rDescriptor
                    reference to the MediaDescriptor (represented by an easy-to-use
                    stl interface!), which should be patched.

        @param      sType
                    the internal type name, where we search a filter for.
                    Used as IN/OUT parameter so we can overrule the detection result for
                    types too !

        @note       #i60158#
                    sometimes our text ascii and our csv filter cant work together.
                    Then we overwrite our detection hardly.
                    sType param is used as out parameter then too ... and
                    rDescriptor will be changed by selecting another filter.
                    (see code)
     */
    void impl_checkResultsAndAddBestFilter(::comphelper::MediaDescriptor& rDescriptor,
                                           ::rtl::OUString&               sType      );

//-------------------------------------------
// uno interface

public:

    //---------------------------------------
    // XTypeDetection

    virtual ::rtl::OUString SAL_CALL queryTypeByURL(const ::rtl::OUString& sURL)
        throw (css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL queryTypeByDescriptor(css::uno::Sequence< css::beans::PropertyValue >& lDescriptor,
                                                           sal_Bool                                         bAllowDeep )
        throw (css::uno::RuntimeException);

//-------------------------------------------
// static uno helper!

public:

    //---------------------------------------

    /** @short  return the uno implementation name of this class.

        @descr  Because this information is used at several places
                (and mostly an object instance of this class is not possible)
                its implemented as a static function!

        @return The fix uno implementation name of this class.
     */
    static ::rtl::OUString impl_getImplementationName();

    //---------------------------------------

    /** @short  return the list of supported uno services of this class.

        @descr  Because this information is used at several places
                (and mostly an object instance of this class is not possible)
                its implemented as a static function!

        @return The fix list of uno services supported by this class.
     */
    static css::uno::Sequence< ::rtl::OUString > impl_getSupportedServiceNames();

    //---------------------------------------

    /** @short  return a new intsnace of this class.

        @descr  This method is used by the uno service manager, to create
                a new instance of this service if needed.

        @param  xSMGR
                reference to the uno service manager, which require
                this new instance. It should be passed to the new object
                so it can be used internaly to create own needed uno resources.

        @return The new instance of this service as an uno reference.
     */
    static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

}}

#endif // __FILTER_CONFIG_TYPEDETECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
