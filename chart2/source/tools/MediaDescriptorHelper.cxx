/*************************************************************************
 *
 *  $RCSfile: MediaDescriptorHelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:32 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "MediaDescriptorHelper.hxx"

using namespace ::com::sun::star;

namespace apphelper
{


const short FLAG_DEPRECATED =1;
const short FLAG_MODEL      =2;

#define WRITE_PROPERTY( MediaName, nFlags )             \
if(rProp.Name.equals(::rtl::OUString::createFromAscii(#MediaName))) \
{                                                       \
    if( rProp.Value >>= MediaName )                     \
        ISSET_##MediaName = sal_True;                   \
    if(nFlags & FLAG_DEPRECATED)                        \
    {                                                   \
        m_aDeprecatedProperties[nDeprecatedCount]=rProp;\
        nDeprecatedCount++;                             \
    }                                                   \
    else                                                \
    {                                                   \
        m_aRegularProperties[nRegularCount]=rProp;      \
        nRegularCount++;                                \
        if( nFlags & FLAG_MODEL)                        \
        {                                               \
            m_aModelProperties[nModelCount]=rProp;      \
            nModelCount++;                              \
        }                                               \
    }                                                   \
}

MediaDescriptorHelper::MediaDescriptorHelper( const uno::Sequence<
                        beans::PropertyValue > & rMediaDescriptor )
{
    impl_init();

    m_aRegularProperties.realloc(0);
    m_aRegularProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nRegularCount = 0;

    m_aDeprecatedProperties.realloc(0);
    m_aDeprecatedProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nDeprecatedCount = 0;

    m_aAdditionalProperties.realloc(0);
    m_aAdditionalProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nAdditionalCount = 0;

    m_aModelProperties.realloc(0);
    m_aModelProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nModelCount = 0;


    //read given rMediaDescriptor and store in internal structures:
    for( sal_Int32 i= rMediaDescriptor.getLength();i--;)
    {
        const beans::PropertyValue& rProp = rMediaDescriptor[i];
        WRITE_PROPERTY( AsTemplate, FLAG_MODEL )
        else WRITE_PROPERTY( Author, FLAG_MODEL )
        else WRITE_PROPERTY( CharacterSet, FLAG_MODEL )
        else WRITE_PROPERTY( Comment, FLAG_MODEL )
        else WRITE_PROPERTY( ComponentData, FLAG_MODEL )
        else WRITE_PROPERTY( FileName, FLAG_DEPRECATED )
        else WRITE_PROPERTY( FilterData, FLAG_MODEL )
        else WRITE_PROPERTY( FilterName, FLAG_MODEL )
        else WRITE_PROPERTY( FilterFlags, FLAG_DEPRECATED)
        else WRITE_PROPERTY( FilterOptions, FLAG_MODEL )
        else WRITE_PROPERTY( FrameName, FLAG_MODEL )
        else WRITE_PROPERTY( Hidden, FLAG_MODEL )
        else WRITE_PROPERTY( InputStream, 0 )
        else WRITE_PROPERTY( InteractionHandler, 0 )
        else WRITE_PROPERTY( JumpMark, 0 )
        else WRITE_PROPERTY( MediaType, FLAG_MODEL )
        else WRITE_PROPERTY( OpenFlags, FLAG_DEPRECATED )
        else WRITE_PROPERTY( OpenNewView, 0 )
        else WRITE_PROPERTY( Overwrite, FLAG_MODEL )
        else WRITE_PROPERTY( Password, FLAG_MODEL )
        else WRITE_PROPERTY( PosSize, 0 )
        else WRITE_PROPERTY( PostData, 0 )
        else WRITE_PROPERTY( PostString, FLAG_DEPRECATED )
        else WRITE_PROPERTY( Preview, FLAG_MODEL )
        else WRITE_PROPERTY( ReadOnly, 0 )
        else WRITE_PROPERTY( Referer, FLAG_MODEL )
        else WRITE_PROPERTY( Silent, 0 )
        else WRITE_PROPERTY( StatusIndicator, 0 )
        else WRITE_PROPERTY( TemplateName, FLAG_DEPRECATED )
        else WRITE_PROPERTY( TemplateRegionName, FLAG_DEPRECATED )
        else WRITE_PROPERTY( Unpacked, FLAG_MODEL )
        else WRITE_PROPERTY( URL, FLAG_MODEL )
        else WRITE_PROPERTY( Version, FLAG_MODEL )
        else WRITE_PROPERTY( ViewData, FLAG_MODEL )
        else WRITE_PROPERTY( ViewId, FLAG_MODEL )
        else
        {
            m_aAdditionalProperties[nAdditionalCount]=rProp;
            nAdditionalCount++;
        }
    }

    m_aRegularProperties.realloc(nRegularCount);
    m_aDeprecatedProperties.realloc(nDeprecatedCount);
    m_aAdditionalProperties.realloc(nAdditionalCount);
    m_aModelProperties.realloc(nModelCount);
}

void MediaDescriptorHelper::impl_init()
{
    AsTemplate = sal_False;
    ISSET_AsTemplate = sal_False;

    ISSET_Author = sal_False;
    ISSET_CharacterSet = sal_False;
    ISSET_Comment = sal_False;

//  ::com::sun::star::uno::Any  ComponentData;
    ISSET_ComponentData = sal_False;
    ISSET_FileName = sal_False;

//  ::com::sun::star::uno::Any  FilterData;
    ISSET_FilterData = sal_False;
    ISSET_FilterName = sal_False;
    ISSET_FilterFlags = sal_False;
    ISSET_FilterOptions = sal_False;
    ISSET_FrameName = sal_False;

    Hidden = sal_False;
    ISSET_Hidden = sal_False;
    ISSET_InputStream = sal_False;
    ISSET_InteractionHandler = sal_False;
    ISSET_JumpMark = sal_False;
    ISSET_MediaType = sal_False;
    ISSET_OpenFlags = sal_False;
    OpenNewView = sal_False;
    ISSET_OpenNewView = sal_False;
    Overwrite = sal_False;
    ISSET_Overwrite = sal_False;
    ISSET_Password = sal_False;

//  ::com::sun::star::awt::Rectangle PosSize;
    ISSET_PosSize = sal_False;

//  ::com::sun::star::uno::Sequence< sal_Int8 > PostData;
    ISSET_PostData = sal_False;
    ISSET_PostString = sal_False;
    Preview = sal_False;
    ISSET_Preview = sal_False;
    ReadOnly = sal_False;
    ISSET_ReadOnly = sal_False;
    ISSET_Referer = sal_False;
    ISSET_StatusIndicator = sal_False;
    Silent = sal_False;
    ISSET_Silent = sal_False;
    ISSET_TemplateName = sal_False;
    ISSET_TemplateRegionName = sal_False;
    Unpacked = sal_False;
    ISSET_Unpacked = sal_False;
    ISSET_URL = sal_False;
    Version = 0;
    ISSET_Version = sal_False;

//  ::com::sun::star::uno::Any ViewData;
    ISSET_ViewData = sal_False;
    ViewId = 0;
    ISSET_ViewId = sal_False;
}

MediaDescriptorHelper::~MediaDescriptorHelper()
{

}

    uno::Sequence< beans::PropertyValue > MediaDescriptorHelper
::getAll()
{
    uno::Sequence< beans::PropertyValue > aAll( m_aRegularProperties );
    //write properties to aAll
    if( m_aDeprecatedProperties.getLength() )
    {
        sal_Int32 nCount = m_aRegularProperties.getLength();
        m_aRegularProperties.realloc( nCount + m_aDeprecatedProperties.getLength());
        for(sal_Int32 i=0;i<m_aDeprecatedProperties.getLength();i++)
            aAll[nCount+i]=m_aDeprecatedProperties[i];
    }
    if( m_aAdditionalProperties.getLength() )
    {
        sal_Int32 nCount = m_aRegularProperties.getLength();
        m_aRegularProperties.realloc(nCount + m_aAdditionalProperties.getLength());
        for(sal_Int32 i=0;i<m_aAdditionalProperties.getLength();i++)
            aAll[nCount+i]=m_aAdditionalProperties[i];
    }
    return aAll;
}
    uno::Sequence< beans::PropertyValue > MediaDescriptorHelper
::getReducedForModel()
{
    return m_aModelProperties;
}
}
