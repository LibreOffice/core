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


#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_CONFIGURATIONACCESS_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_CONFIGURATIONACCESS_HXX
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
    OUString   maName;
    bool        mbJPEGCompression;
    sal_Int32       mnJPEGQuality;
    bool        mbRemoveCropArea;
    sal_Int32       mnImageResolution;
    bool        mbEmbedLinkedGraphics;
    bool        mbOLEOptimization;
    sal_Int16       mnOLEOptimizationType;
    bool        mbDeleteUnusedMasterPages;
    bool        mbDeleteHiddenSlides;
    bool        mbDeleteNotesPages;
    OUString   maCustomShowName;
    bool        mbSaveAs;
    OUString   maSaveAsURL;
    OUString   maFilterName;
    bool        mbOpenNewDocument;
    sal_Int64       mnEstimatedFileSize;

    OptimizerSettings() :
        mbJPEGCompression( false ),
        mnJPEGQuality( 90 ),
        mbRemoveCropArea( false ),
        mnImageResolution( 0 ),
        mbEmbedLinkedGraphics( false ),
        mbOLEOptimization( false ),
        mnOLEOptimizationType( 0 ),
        mbDeleteUnusedMasterPages( false ),
        mbDeleteHiddenSlides( false ),
        mbDeleteNotesPages( false ),
        mbSaveAs( true ),
        mbOpenNewDocument( true ),
        mnEstimatedFileSize( 0 ){};
        ~OptimizerSettings(){};

        void LoadSettingsFromConfiguration( const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& rSettings );
        void SaveSettingsToConfiguration( const com::sun::star::uno::Reference< com::sun::star::container::XNameReplace >& rSettings );

        bool operator==( const OptimizerSettings& rOptimizerSettings ) const;

};
class ConfigurationAccess
{
    public :

        ConfigurationAccess( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rXFactory,
                                OptimizerSettings* pDefaultSettings = NULL );
        ~ConfigurationAccess();
        void SaveConfiguration();

        OUString getPath( OUString const & token );
        OUString getString( const PPPOptimizerTokenEnum ) const;

        // access to current OptimizerSettings (stored in the first entry of maSettings)
        com::sun::star::uno::Any GetConfigProperty( const PPPOptimizerTokenEnum ) const;
        void SetConfigProperty( const PPPOptimizerTokenEnum, const com::sun::star::uno::Any& aValue );

        bool GetConfigProperty( const PPPOptimizerTokenEnum, const bool bDefault ) const;
        sal_Int16 GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Int16 nDefault ) const;
        sal_Int32 GetConfigProperty( const PPPOptimizerTokenEnum, const sal_Int32 nDefault ) const;

        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetConfigurationSequence();

        // getting access to the OptimizerSettings list
        std::vector< OptimizerSettings >& GetOptimizerSettings() { return maSettings; };
        std::vector< OptimizerSettings >::iterator GetOptimizerSettingsByName( const OUString& rName );

    private :

        struct Compare
        {
            bool operator()( const PPPOptimizerTokenEnum s1, const PPPOptimizerTokenEnum s2 ) const
            {
                return s1 < s2;
            }
        };
        std::map < PPPOptimizerTokenEnum, OUString, Compare > maStrings;

        std::vector< OptimizerSettings > maSettings;
        std::vector< OptimizerSettings > maInitialSettings;

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;

        void LoadStrings();
        void LoadConfiguration();
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface > OpenConfiguration( bool bReadOnly );
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface > GetConfigurationNode(
            const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& xRoot, const OUString& sPathToNode );
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_CONFIGURATIONACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
