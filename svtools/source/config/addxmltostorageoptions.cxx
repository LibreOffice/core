/*************************************************************************
 *
 *  $RCSfile: addxmltostorageoptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 10:06:01 $
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
#ifndef GCC
#pragma hdrstop
#endif

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "addxmltostorageoptions.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtAddXMLToStorageOptions_Impl* SvtAddXMLToStorageOptions::m_pDataContainer = 0;
sal_Int32 SvtAddXMLToStorageOptions::m_nRefCount = 0;


//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtAddXMLToStorageOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtAddXMLToStorageOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      access method to get internal values
            @descr      These method give us a chance to regulate acces to ouer internal values.
                        It's not used in the moment - but it's possible for the feature!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool IsWriter_Add_XML_to_Storage() const    { return bAddXmlToStg_Writer; }
        sal_Bool IsCalc_Add_XML_to_Storage() const      { return bAddXmlToStg_Calc; }
        sal_Bool IsImpress_Add_XML_to_Storage() const   { return bAddXmlToStg_Impress; }
        sal_Bool IsDraw_Add_XML_to_Storage() const      { return bAddXmlToStg_Draw; }

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of ouer configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_Bool bAddXmlToStg_Writer,
                 bAddXmlToStg_Calc,
                 bAddXmlToStg_Impress,
                 bAddXmlToStg_Draw;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________


//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtAddXMLToStorageOptions_Impl::SvtAddXMLToStorageOptions_Impl()
    // Init baseclasses first
    :   ConfigItem( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                        "Office.Common/AddXMLToStorage"))),
    // Init member then.
    bAddXmlToStg_Writer( FALSE ),
    bAddXmlToStg_Calc( FALSE ),
    bAddXmlToStg_Impress( FALSE ),
    bAddXmlToStg_Draw( FALSE )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString > seqNames = GetPropertyNames();
    Sequence< Any > seqValues = GetProperties( seqNames );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    const Any* pValue = seqValues.getConstArray();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty, ++pValue )
        if( pValue->hasValue() )

            switch( nProperty )
            {
            case 0:
                *pValue >>= bAddXmlToStg_Writer;
                break;
            case 1:
                *pValue >>= bAddXmlToStg_Calc;
                break;
            case 2:
                *pValue >>= bAddXmlToStg_Impress;
                break;
            case 3:
                *pValue >>= bAddXmlToStg_Draw;
                break;
            }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtAddXMLToStorageOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    static const sal_Char* pProperties[] =
    {
        "Writer",
        "Calc",
        "Impress",
        "Draw"
    };

    const sal_uInt16 nCnt = sizeof(pProperties) / sizeof( pProperties[0] );
    Sequence<OUString> aNames( nCnt );
    OUString* pNames = aNames.getArray();
    for( sal_uInt16 n = 0; n < nCnt; ++n )
        pNames[ n ] = OUString::createFromAscii( pProperties[ n ] );
    return aNames;
}


//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtAddXMLToStorageOptions::SvtAddXMLToStorageOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( !m_pDataContainer )
        m_pDataContainer = new SvtAddXMLToStorageOptions_Impl;
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtAddXMLToStorageOptions::~SvtAddXMLToStorageOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( !--m_nRefCount )
        delete m_pDataContainer, m_pDataContainer = 0;
}

sal_Bool SvtAddXMLToStorageOptions::IsWriter_Add_XML_to_Storage() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsWriter_Add_XML_to_Storage();
}
sal_Bool SvtAddXMLToStorageOptions::IsCalc_Add_XML_to_Storage() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsCalc_Add_XML_to_Storage();
}
sal_Bool SvtAddXMLToStorageOptions::IsImpress_Add_XML_to_Storage() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsImpress_Add_XML_to_Storage();
}
sal_Bool SvtAddXMLToStorageOptions::IsDraw_Add_XML_to_Storage() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsDraw_Add_XML_to_Storage();
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtAddXMLToStorageOptions::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}
