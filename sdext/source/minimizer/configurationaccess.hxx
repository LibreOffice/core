/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configurationaccess.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:49:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _CONFIGURATION_ACCESS_HXX_
#define _CONFIGURATION_ACCESS_HXX_
#include <vector>
#ifndef _PPPOPTIMIZER_TOKEN_HXX
#include "pppoptimizertoken.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#include <map>

struct OptimizerSettings
{
    rtl::OUString   maName;
    sal_Bool        mbJPEGCompression;
    sal_Int32       mnJPEGQuality;
    sal_Bool        mbRemoveCropArea;
    sal_Int32       mnImageResolution;
    sal_Bool        mbEmbedLinkedGraphics;
    sal_Bool        mbOLEOptimization;
    sal_Int16       mnOLEOptimizationType;
    sal_Bool        mbDeleteUnusedMasterPages;
    sal_Bool        mbDeleteHiddenSlides;
    sal_Bool        mbDeleteNotesPages;
    rtl::OUString   maCustomShowName;
    sal_Bool        mbSaveAs;
    rtl::OUString   maSaveAsURL;
    rtl::OUString   maFilterName;
    sal_Bool        mbOpenNewDocument;

    OptimizerSettings() :
        mbJPEGCompression( sal_False ),
        mnJPEGQuality( 90 ),
        mbRemoveCropArea( sal_False ),
        mnImageResolution( 0 ),
        mbEmbedLinkedGraphics( sal_False ),
        mbOLEOptimization( sal_False ),
        mnOLEOptimizationType( 0 ),
        mbDeleteUnusedMasterPages( sal_False ),
        mbDeleteHiddenSlides( sal_False ),
        mbDeleteNotesPages( sal_False ),
        mbSaveAs( sal_True ),
        mbOpenNewDocument( sal_True ){};
        ~OptimizerSettings(){};

        void LoadSettingsFromConfiguration( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rSettings );
        void SaveSettingsToConfiguration( const com::sun::star::uno::Reference< com::sun::star::container::XNameReplace >& rSettings );

        sal_Bool operator==( const OptimizerSettings& rOptimizerSettings ) const;

};
class ConfigurationAccess
{
    public :

        ConfigurationAccess( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rXFactory,
                                OptimizerSettings* pDefaultSettings = NULL );
        ~ConfigurationAccess();
        void SaveConfiguration();

        rtl::OUString getPath( const PPPOptimizerTokenEnum );
        rtl::OUString getString( const PPPOptimizerTokenEnum ) const;

        // access to current OptimizerSettings (stored in the first entry of maSettings)
        com::sun::star::uno::Any GetConfigProperty( const PPPOptimizerTokenEnum ) const;
        void SetConfigProperty( const PPPOptimizerTokenEnum, const com::sun::star::uno::Any& aValue );

        sal_Bool GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Bool bDefault ) const;
        sal_Int16 GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Int16 nDefault ) const;
        sal_Int32 GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Int32 nDefault ) const;

        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetConfigurationSequence();

        // getting access to the OptimizerSettings list
        std::vector< OptimizerSettings >& GetOptimizerSettings() { return maSettings; };
        std::vector< OptimizerSettings >::iterator GetOptimizerSettingsByName( const rtl::OUString& rName );

    private :

        struct Compare
        {
            bool operator()( const PPPOptimizerTokenEnum s1, const PPPOptimizerTokenEnum s2 ) const
            {
                return s1 < s2;
            }
        };
        std::map < PPPOptimizerTokenEnum, rtl::OUString, Compare > maStrings;

        std::vector< OptimizerSettings > maSettings;
        std::vector< OptimizerSettings > maInitialSettings;

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxMSF;

        void LoadStrings();
        void LoadConfiguration();
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface > OpenConfiguration( bool bReadOnly );
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface > GetConfigurationNode(
            const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& xRoot, const rtl::OUString& sPathToNode );
};

#endif  // _CONFIGURATION_ACCESS_HXX_

