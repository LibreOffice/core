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

#ifndef SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX
#define SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <salhelper/thread.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>
#include <vcl/image.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= SortingData_Impl
    //====================================================================
    struct SortingData_Impl
    {
    private:
        OUString maFilename;     // only filename in upper case - for compare purposes
        OUString maTitle;        //  -> be carefull when changing maTitle to update maFilename only when new
        OUString maLowerTitle;


    public:
        OUString maType;
        OUString maTargetURL;
        OUString maImageURL;
        OUString maDisplayText;
        DateTime        maModDate;
        Image           maImage;
        sal_Int64       maSize;
        sal_Bool        mbIsFolder;
        sal_Bool        mbIsVolume;
        sal_Bool        mbIsRemote;
        sal_Bool        mbIsRemoveable;
        sal_Bool        mbIsFloppy;
        sal_Bool        mbIsCompactDisc;

        inline                          SortingData_Impl();
        inline const OUString&   GetTitle() const;
        inline const OUString&   GetLowerTitle() const;
        inline const OUString&   GetFileName() const;
        inline void                     SetNewTitle( const OUString& rNewTitle );        // new maTitle is set -> maFilename is set to same!
        inline void                     ChangeTitle( const OUString& rChangedTitle );    // maTitle is changed, maFilename is unchanged!

    private:
        inline void                     SetTitles( const OUString& rNewTitle );
    };

    inline SortingData_Impl::SortingData_Impl() :
        maModDate       ( DateTime::EMPTY ),
        maSize          ( 0 ),
        mbIsFolder      ( sal_False ),
        mbIsVolume      ( sal_False ),
        mbIsRemote      ( sal_False ),
        mbIsRemoveable  ( sal_False ),
        mbIsFloppy      ( sal_False ),
        mbIsCompactDisc ( sal_False )
    {
    }

    inline const OUString& SortingData_Impl::GetTitle() const
    {
        return maTitle;
    }

    inline const OUString& SortingData_Impl::GetLowerTitle() const
    {
        return maLowerTitle;
    }

    inline const OUString& SortingData_Impl::GetFileName() const
    {
        return maFilename;
    }

    inline void SortingData_Impl::SetNewTitle( const OUString& rNewTitle )
    {
        SetTitles( rNewTitle );
        maFilename = rNewTitle.toAsciiUpperCase();
    }

    inline void SortingData_Impl::ChangeTitle( const OUString& rChangedTitle )
    {
        SetTitles( rChangedTitle );
    }

    inline void SortingData_Impl::SetTitles( const OUString& rNewTitle )
    {
        maTitle = rNewTitle;
        maLowerTitle = rNewTitle.toAsciiLowerCase();
    }

    //====================================================================
    //= IContentTitleTranslation
    //====================================================================
    class IContentTitleTranslation
    {
    public:
        virtual sal_Bool    GetTranslation( const OUString& _rOriginalName, OUString& _rTranslatedName ) const = 0;

    protected:
        ~IContentTitleTranslation() {}
    };

    //====================================================================
    //= EnumerationResult
    //====================================================================
    enum EnumerationResult
    {
        SUCCESS,    /// the enumeration was successful
        ERROR,      /// the enumeration was unsuccessful
        RUNNING     /// the enumeration is still running, and the maximum wait time has passed
    };

    //====================================================================
    //= FolderDescriptor
    //====================================================================
    struct FolderDescriptor
    {
        /** a content object describing the folder. Can be <NULL/>, in this case <member>sURL</member>
            is relevant.
        */
        ::ucbhelper::Content  aContent;
        /** the URL of a folder. Will be ignored if <member>aContent</member> is not <NULL/>.
        */
        OUString          sURL;

        FolderDescriptor() { }

        FolderDescriptor( const ::ucbhelper::Content& _rContent )
            :aContent( _rContent )
        {
        }

        FolderDescriptor( const OUString& _rURL )
            :sURL( _rURL )
        {
        }
    };

    //====================================================================
    //= IEnumerationResultHandler
    //====================================================================
    class IEnumerationResultHandler
    {
    public:
        virtual void        enumerationDone( EnumerationResult _eResult ) = 0;

    protected:
        ~IEnumerationResultHandler() {}
    };

    //====================================================================
    //= FileViewContentEnumerator
    //====================================================================
    class FileViewContentEnumerator: public salhelper::Thread
    {
    public:
        typedef ::std::vector< SortingData_Impl* >  ContentData;

    private:
        ContentData&                    m_rContent;
        ::osl::Mutex&                   m_rContentMutex;

        mutable ::osl::Mutex            m_aMutex;

        FolderDescriptor                m_aFolder;
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >
                                        m_xCommandEnv;
        const IContentTitleTranslation* m_pTranslator;
        IEnumerationResultHandler*      m_pResultHandler;
        bool                            m_bCancelled;

        mutable ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties>
                                        m_xDocProps;

        ::com::sun::star::uno::Sequence< OUString > m_rBlackList;

        sal_Bool URLOnBlackList ( const OUString& sRealURL );

    public:
        /** constructs an enumerator instance

            @param _rContentToFill
                the structure which is to be filled with the found content
            @param _rContentMutex
                the mutex which protects the access to <arg>_rContentToFill</arg>
            @param _pTranslator
                an instance which should be used to translate content titles. May be <NULL/>
        */
        FileViewContentEnumerator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& _rxCommandEnv,
            ContentData& _rContentToFill,
            ::osl::Mutex& _rContentMutex,
            const IContentTitleTranslation* _pTranslator
        );

        /** enumerates the content of a given folder

            @param _rFolder
                the folder whose content is to be enumerated
            @param _pFilter
                a filter to apply to the found contents
            @param _pResultHandler
                an instance which should handle the results of the enumeration
        */
        void    enumerateFolderContent(
                    const FolderDescriptor& _rFolder,
                    IEnumerationResultHandler* _pResultHandler
                );

        /** enumerates the content of a given folder synchronously
        */
        EnumerationResult   enumerateFolderContentSync(
                    const FolderDescriptor& _rFolder,
                    const ::com::sun::star::uno::Sequence< OUString >& rBlackList = ::com::sun::star::uno::Sequence< OUString >()
                );

        /** cancels the running operation.

            Note that "cancel" may mean that the operation is running, but its result
            is simply disregarded later on.
        */
        void    cancel();

    protected:
        ~FileViewContentEnumerator();

    private:
        EnumerationResult enumerateFolderContent();

        // Thread overridables
        virtual void execute();

    private:
        sal_Bool implGetDocTitle( const OUString& _rTargetURL, OUString& _rRet ) const;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
