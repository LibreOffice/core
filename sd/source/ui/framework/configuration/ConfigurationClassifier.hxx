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

#ifndef SD_FRAMEWORK_CONFIGURATION_CLASSIFIER_HXX
#define SD_FRAMEWORK_CONFIGURATION_CLASSIFIER_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>

#include <vector>

namespace sd { namespace framework {

/** A ConfigurationClassifier object compares two configurations of
    resources and gives access to the differences.  It is used mainly when
    changes to the current configuration have been requested and the various
    resource controllers have to be supplied with the set of resources that
    are to be activated or deactivated.
*/
class ConfigurationClassifier
{
public:
    /** Create a new ConfigurationClassifier object that will compare the
        two given configurations.
    */
    ConfigurationClassifier (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration1,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration2);

    /** Calculate three lists of resource ids.  These contain the resources
        that belong to one configuration but not the other, or that belong
        to both configurations.
        @return
            When the two configurations differ then return <TRUE/>.  When
            they are equivalent then return <FALSE/>.
    */
    bool Partition (void);

    typedef ::std::vector<com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId> > ResourceIdVector;

    /** Return the resources that belong to the configuration given as
        rxConfiguration1 to the constructor but that do not belong to
        rxConfiguration2.
        @return
            A reference to the, possibly empty, list of resources is
            returned.  This reference remains valid as long as the called
            ConfigurationClassifier object stays alive.
    */
    const ResourceIdVector& GetC1minusC2 (void) const;

    /** Return the resources that belong to the configuration given as
        rxConfiguration2 to the constructor but that do not belong to
        rxConfiguration1.
        @return
            A reference to the, possibly empty, list of resources is
            returned.  This reference remains valid as long as the called
            ConfigurationClassifier object stays alive.
    */
    const ResourceIdVector& GetC2minusC1 (void) const;

    /** Return the resources that belong to both the configurations that
        where given to the constructor.
        @return
            A reference to the, possibly empty, list of resources is
            returned.  This reference remains valid as long as the called
            ConfigurationClassifier object stays alive.
    */
    const ResourceIdVector& GetC1andC2 (void) const;

    void TraceResourceIdVector (
        const sal_Char* pMessage,
        const ResourceIdVector& rResources) const;

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration> mxConfiguration1;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration> mxConfiguration2;

    /** After the call to Classify() this vector holds all elements from
        mxConfiguration1 that are not in mxConfiguration2.
    */
    ResourceIdVector maC1minusC2;

    /** After the call to Classify() this vector holds all elements from
        mxConfiguration2 that are not in mxConfiguration1.
    */
    ResourceIdVector maC2minusC1;

    /** After the call to Classify() this vector holds all elements that are
        member both of mxConfiguration1 and mxConfiguration2.
    */
    ResourceIdVector maC1andC2;

    /** Put all the elements in the two gven sequences of resource ids and
        copy them into one of the resource id result vectors maC1minusC2,
        maC2minusC1, and maC1andC2.  This is done by using only the resource
        URLs for classification.  Therefor this method calls itself
        recursively.
        @param rS1
            One sequence of XResourceId objects.
        @param rS2
            Another sequence of XResourceId objects.
    */
    void PartitionResources (
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::framework::XResourceId> >& rS1,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::framework::XResourceId> >& rS2);

    /** Compare the given sequences of resource ids and put their elements
        in one of three vectors depending on whether an element belongs to
        both sequences or to one but not the other.  Note that only the
        resource URLs of the XResourceId objects are used for the
        classification.
        @param rS1
            One sequence of XResourceId objects.
        @param rS2
            Another sequence of XResourceId objects.
    */
    void ClassifyResources (
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::framework::XResourceId> >& rS1,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::framework::XResourceId> >& rS2,
        ResourceIdVector& rS1minusS2,
        ResourceIdVector& rS2minusS1,
        ResourceIdVector& rS1andS2);


    /** Copy the resources given in rSource to the list of resources
        specified by rTarget.  Resources bound to the ones in rSource,
        either directly or indirectly, are copied as well.
        @param rSource
            All resources and the ones bound to them, either directly or
            indirectly, are copied.
        @param rxConfiguration
            This configuration is used to determine the resources bound to
            the ones in rSource.
        @param rTarget
            This list is filled with resources from rSource and the ones
            bound to them.
    */
    void CopyResources (
        const ResourceIdVector& rSource,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration,
        ResourceIdVector& rTarget);
};

} } // end of namespace sd::framework

#endif
