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
#ifndef _XMLOFF_CONDITIONALMULTIPROPERTYSETHELPER_HXX
#define _XMLOFF_CONDITIONALMULTIPROPERTYSETHELPER_HXX


#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


namespace com { namespace sun { namespace star {
    namespace beans { class XMultiPropertySet; }
    namespace beans { class XPropertySet; }
    namespace beans { class XPropertySetInfo; }
} } }
namespace binfilter {


/**
 * The MultiPropertySetHelper performs the follwing functions:
 *
 * Given a list of property names (as sal_Char** or OUString*), it can
 * query an XMultiPropertySet (or XPropertySet) which of these properties
 * it supports (method hasProperties(...)). The properties *MUST* be
 * sorted alphabetically.
 *
 * Then, the X(Multi)PropertySet can be queried for values, and only
 * the supported properties are queried. (method getValues(...)) The
 * values are stored in the helper itself.
 *
 * Finally, each property can be queried for existence 
 * (method hasProperty(...)) or its value (method (getValue(...))).
 *
 * After some initial preparation (hasProperties, getValues) the
 * MultiPropertySetHelper can be used similarly to an 
 * XPropertySet in that you can query the values in the places where you
 * need them. However, if an XMultiPropertySet is supplied, the queries
 * are more efficient, often significantly so.
 */
class MultiPropertySetHelper
{
    /// names of all properties
    ::rtl::OUString* pPropertyNames;

    /// length of pPropertyNames array
    sal_Int16 nLength;

    /// the sequence of property names that the current (multi)
    /// property set implementation supports
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aPropertySequence;

    /// an array of indices that maps from pPropertyNames indices to
    /// aPropertySequence indices
    sal_Int16* pSequenceIndex;

    /// the last set of values retrieved by getValues
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aValues;

    /// result of aValues.getConstArray()
    const ::com::sun::star::uno::Any* pValues;

    /// an empty Any
    ::com::sun::star::uno::Any aEmptyAny;

public:

    MultiPropertySetHelper( const sal_Char** pNames );

    ~MultiPropertySetHelper();


    /**
     * Call hasPropertiesByName for the provided XPropertySetInfo and build
     * list of allowed properties.
     */
    void hasProperties( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySetInfo> & );


    /**
     * Return whether hasProperties was called
     * (i.e. if we are ready to call getValues)
     */
    sal_Bool checkedProperties();

    /**
     * Get values from the XMultiPropertySet.
     *
     * May only be called after hasProperties() was called for the
     * appropriate XPropertySetInfo.
     */
    void getValues( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XMultiPropertySet> & );

    /** 
     * Get values from the XPropertySet. This can be much slower than
     * getValues( const Reference<XMultiPropertySet& ) and hence
     * should be avoided. 
     *
     * May only be called after hasProperties() was called for the
     * appropriate XPropertySetInfo. 
     */
    void getValues( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet> & );



    /**
     * Get a value from the values array.
     *
     * May only be called after getValues() was called.
     */
    inline const ::com::sun::star::uno::Any& getValue( sal_Int16 nIndex );

    /**
     * Find out if this property is supported.
     *
     * May only be called after hasProperties() was called.
     */
    inline sal_Bool hasProperty( sal_Int16 nIndex );

    /**
     * Get a value from the XPropertySet on demand. 
     *
     * If neither getValues nor getValueOnDemand has been called already
     * after the last call to resetValues, the values are retrieved
     * using getValues. Otherwise the value already retrieved is returned.
     * In case XMultiPropertySet is supported by the XPropertySet and
     * bTryMult is set, the XMultiPropertySet is used to get the values.
     * 
     */
    const ::com::sun::star::uno::Any& getValue( sal_Int16 nIndex,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet> &,
                        sal_Bool bTryMulti = sal_False );

    /**
     * Get a value from the XMultiPropertySet on demand. 
     *
     * If neither getValues nor getValueOnDemand has been called already
     * after the last call to resetValues, the values are retrieved
     * using getValues. Otherwise the value already retrieved is returned.
     * In case XMultiPropertySet is supported by the XPropertySet,
     * XMultiPropertySet is used to get the values.
     * 
     */
    const ::com::sun::star::uno::Any& getValue( sal_Int16 nIndex,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XMultiPropertySet> & );

    inline void resetValues() { pValues = 0; }
};


// inline implementations of the often-called methods getValue and hasProperty:

const ::com::sun::star::uno::Any& MultiPropertySetHelper::getValue(
    sal_Int16 nValueNo )
{
    DBG_ASSERT( pValues != NULL, 
                "called getValue() without calling getValues() before");
    DBG_ASSERT( pSequenceIndex != NULL,
                "called getValue() without calling hasProperties() before" );
    DBG_ASSERT( nValueNo < nLength, "index out of range" );

    sal_Int16 nIndex = pSequenceIndex[ nValueNo ];
    return ( nIndex != -1 ) ? pValues[ nIndex ] : aEmptyAny;
}

sal_Bool MultiPropertySetHelper::hasProperty( sal_Int16 nValueNo )
{
    DBG_ASSERT( pSequenceIndex != NULL,
                "called getValue() without calling hasProperties() before" );
    DBG_ASSERT( nValueNo < nLength, "index out of range" );

    return pSequenceIndex[ nValueNo ] != -1;
}

}//end of namespace binfilter
#endif
