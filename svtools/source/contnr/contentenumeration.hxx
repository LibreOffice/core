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

#ifndef SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX
#define SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#include <salhelper/thread.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <vcl/image.hxx>

class IUrlFilter;
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
        ::rtl::OUString maFilename;     // only filename in upper case - for compare purposes
        ::rtl::OUString maTitle;        //  -> be carefull when changing maTitle to update maFilename only when new
        ::rtl::OUString maLowerTitle;


    public:
        ::rtl::OUString maType;
        ::rtl::OUString maTargetURL;
        ::rtl::OUString maImageURL;
        ::rtl::OUString maDisplayText;
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
        inline const ::rtl::OUString&   GetTitle() const;
        inline const ::rtl::OUString&   GetLowerTitle() const;
        inline const ::rtl::OUString&   GetFileName() const;
        inline void                     SetNewTitle( const ::rtl::OUString& rNewTitle );        // new maTitle is set -> maFilename is set to same!
        inline void                     ChangeTitle( const ::rtl::OUString& rChangedTitle );    // maTitle is changed, maFilename is unchanged!

    private:
        inline void                     SetTitles( const ::rtl::OUString& rNewTitle );
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

    inline const ::rtl::OUString& SortingData_Impl::GetTitle() const
    {
        return maTitle;
    }

    inline const ::rtl::OUString& SortingData_Impl::GetLowerTitle() const
    {
        return maLowerTitle;
    }

    inline const ::rtl::OUString& SortingData_Impl::GetFileName() const
    {
        return maFilename;
    }

    inline void SortingData_Impl::SetNewTitle( const ::rtl::OUString& rNewTitle )
    {
        SetTitles( rNewTitle );
        maFilename = rNewTitle.toAsciiUpperCase();
    }

    inline void SortingData_Impl::ChangeTitle( const ::rtl::OUString& rChangedTitle )
    {
        SetTitles( rChangedTitle );
    }

    inline void SortingData_Impl::SetTitles( const ::rtl::OUString& rNewTitle )
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
        virtual sal_Bool    GetTranslation( const ::rtl::OUString& _rOriginalName, ::rtl::OUString& _rTranslatedName ) const = 0;

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
        String          sURL;

        FolderDescriptor() { }

        FolderDescriptor( const ::ucbhelper::Content& _rContent )
            :aContent( _rContent )
        {
        }

        FolderDescriptor( const String& _rURL )
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
        const IUrlFilter*               m_pFilter;
        const IContentTitleTranslation* m_pTranslator;
        IEnumerationResultHandler*      m_pResultHandler;
        bool                            m_bCancelled;

        mutable ::com::sun::star::uno::Reference< ::com::sun::star::document::XStandaloneDocumentInfo >
                                        m_xDocInfo;

        ::com::sun::star::uno::Sequence< ::rtl::OUString > m_rBlackList;

        sal_Bool URLOnBlackList ( const ::rtl::OUString& sRealURL );

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
                    const IUrlFilter* _pFilter,
                    IEnumerationResultHandler* _pResultHandler
                );

        /** enumerates the content of a given folder synchronously
        */
        EnumerationResult   enumerateFolderContentSync(
                    const FolderDescriptor& _rFolder,
                    const IUrlFilter* _pFilter,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList = ::com::sun::star::uno::Sequence< ::rtl::OUString >()
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
        sal_Bool implGetDocTitle( const ::rtl::OUString& _rTargetURL, ::rtl::OUString& _rRet ) const;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
