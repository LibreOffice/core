/*************************************************************************
 *
 *  $RCSfile: MediaDescriptorHelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
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
#ifndef _MEDIADESCRIPTORHELPER_HXX
#define _MEDIADESCRIPTORHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

/*
* This class helps to read and write the properties mentioned in the service description
* com.sun.star.document.MediaDescriptor from and to a sequence of PropertyValues.
* Properties that are not mentioned in the service description
* are stored in the member AdditionalProperties.
*
* As an additional feature this helper class can generate a reduced sequence of PropertyValues
* that does not contain properties which are known to be only view relevant. This
* reduced sequence than might be attached to a model directly.
*/

namespace apphelper
{

class MediaDescriptorHelper
{
private:
    //MediaDescriptorHelper(){};
public:
    MediaDescriptorHelper( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue > & rMediaDescriptor );
    virtual ~MediaDescriptorHelper();

    ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue > getAll();
    ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue > getReducedForModel();

public:
    //---------------------
    //all properties given in the constructor are stored in the following three sequences

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >
                        m_aRegularProperties; //these are the properties which are described in service com.sun.star.document.MediaDescriptor and not marked as deprecated

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >
                        m_aDeprecatedProperties; //these are properties which are described in service com.sun.star.document.MediaDescriptor but are marked as deprecated

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >
                        m_aAdditionalProperties; //these are properties which are not described in service com.sun.star.document.MediaDescriptor

    //----------------------
    //properties which should be given to a model are additionally stored in this sequence (not documented properties and deprecated properties are not included!)
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >
                        m_aModelProperties; //these are properties which are not described in service com.sun.star.document.MediaDescriptor


    //@todo define this for debug only, except URL
    sal_Bool            AsTemplate; //document is a template.
    sal_Bool            ISSET_AsTemplate;
    ::rtl::OUString     Author; //
    sal_Bool            ISSET_Author;
    ::rtl::OUString     CharacterSet; //identifier of used character set.
    sal_Bool            ISSET_CharacterSet;
    ::rtl::OUString     Comment;//
    sal_Bool            ISSET_Comment;

    ::com::sun::star::uno::Any
                        ComponentData;//
    sal_Bool            ISSET_ComponentData;
    ::rtl::OUString     FileName; //deprecated, same as url
    sal_Bool            ISSET_FileName;
    ::com::sun::star::uno::Any
                        FilterData;//
    sal_Bool            ISSET_FilterData;
    ::rtl::OUString     FilterName; //internal filter name.
    sal_Bool            ISSET_FilterName;
    ::rtl::OUString     FilterFlags;//deprecated,
    sal_Bool            ISSET_FilterFlags;
    ::rtl::OUString     FilterOptions;//
    sal_Bool            ISSET_FilterOptions;
            //not documented ... @todo remove?
    ::rtl::OUString     FrameName; //name of target frame.
    sal_Bool            ISSET_FrameName;
    sal_Bool            Hidden; //load document, invisible.
    sal_Bool            ISSET_Hidden;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        InputStream; //content of document. //::com::sun::star::io::XInputStream
    sal_Bool            ISSET_InputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        InteractionHandler; //  //::com::sun::star::task::XInteractionHandler
    sal_Bool            ISSET_InteractionHandler;

    ::rtl::OUString     JumpMark;   //specifies the name of a mark within the document where the first view is to position itself.
    sal_Bool            ISSET_JumpMark;
    ::rtl::OUString     MediaType; //mime type.
    sal_Bool            ISSET_MediaType;
    ::rtl::OUString     OpenFlags; //deprecated
    sal_Bool            ISSET_OpenFlags;
    sal_Bool            OpenNewView; //opens a new view for an already loaded document.
    sal_Bool            ISSET_OpenNewView;
    sal_Bool            Overwrite; //opens a new view for an already loaded document.
    sal_Bool            ISSET_Overwrite;
    ::rtl::OUString     Password; //
    sal_Bool            ISSET_Password;

        //not documented ... @todo remove?
    ::com::sun::star::awt::Rectangle
                        PosSize; //position and size of document window.
    sal_Bool            ISSET_PosSize;

    ::com::sun::star::uno::Sequence< sal_Int8 >
                        PostData; //contains the data for HTTP post method as a sequence of bytes.
    sal_Bool            ISSET_PostData;
    ::rtl::OUString     PostString; //deprecated, contains the data for HTTP post method as a sequence of bytes.
    sal_Bool            ISSET_PostString;
    sal_Bool            Preview; //show preview.
    sal_Bool            ISSET_Preview;
    sal_Bool            ReadOnly; //open document readonly.
    sal_Bool            ISSET_ReadOnly;
    ::rtl::OUString     Referer; //name of document referrer.
    sal_Bool            ISSET_Referer;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        StatusIndicator; //  //::com::sun::star::task::XStatusIndicator
    sal_Bool            ISSET_StatusIndicator;
        //not documented ... @todo remove?
    sal_Bool            Silent; //prevents dialogs to query for more information.
    sal_Bool            ISSET_Silent;
    ::rtl::OUString     TemplateName; //deprecated, name of the template instead of the URL.
    sal_Bool            ISSET_TemplateName;
    ::rtl::OUString     TemplateRegionName; //deprecated, name of the region of the template.
    sal_Bool            ISSET_TemplateRegionName;
    sal_Bool            Unpacked;
    sal_Bool            ISSET_Unpacked;
    ::com::sun::star::util::URL
                        URL;// FileName, URL of the document.
    sal_Bool            ISSET_URL;
    sal_Int16           Version; //storage version.
    sal_Bool            ISSET_Version;

    ::com::sun::star::uno::Any
                        ViewData;//
    sal_Bool            ISSET_ViewData;
    sal_Int16           ViewId; //id of the initial view.
    sal_Bool            ISSET_ViewId;

protected:
    void impl_init();
};

}

#endif
