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


#ifndef _CONFIGURATION_ACCESS_HXX_
#define _CONFIGURATION_ACCESS_HXX_
#include <vector>
#include "pppoptimizertoken.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
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
    sal_Int64       mnEstimatedFileSize;

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
        mbOpenNewDocument( sal_True ),
        mnEstimatedFileSize( 0 ){};
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
