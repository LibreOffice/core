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

#ifndef INCLUDED_SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX
#define INCLUDED_SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX

#include <sal/config.h>

#include <memory>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <salhelper/thread.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>

#include <memory>

namespace svt
{


    //= SortingData_Impl

    struct SortingData_Impl
    {
    private:
        OUString    maFilename;     // only filename in upper case - for compare purposes
        OUString    maTitle;        //  -> be careful when changing maTitle to update maFilename only when new
        OUString    maLowerTitle;


    public:
        OUString    maType;
        OUString    maTargetURL;
        OUString    maDisplayName;
        OUString    maDisplaySize;
        OUString    maDisplayDate;
        DateTime    maModDate;
        OUString    maImage;
        sal_Int64   maSize;
        bool        mbIsFolder;
        bool        mbIsVolume;
        bool        mbIsRemote;
        bool        mbIsRemoveable;
        bool        mbIsFloppy;
        bool        mbIsCompactDisc;

        inline                          SortingData_Impl();
        inline const OUString&   GetTitle() const;
        inline const OUString&   GetLowerTitle() const;
        inline const OUString&   GetFileName() const;
        inline void                     SetNewTitle( const OUString& rNewTitle );        // new maTitle is set -> maFilename is set to same!

    private:
        inline void                     SetTitles( const OUString& rNewTitle );
    };

    inline SortingData_Impl::SortingData_Impl() :
        maModDate       ( DateTime::EMPTY ),
        maSize          ( 0 ),
        mbIsFolder      ( false ),
        mbIsVolume      ( false ),
        mbIsRemote      ( false ),
        mbIsRemoveable  ( false ),
        mbIsFloppy      ( false ),
        mbIsCompactDisc ( false )
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

    inline void SortingData_Impl::SetTitles( const OUString& rNewTitle )
    {
        maTitle = rNewTitle;
        maLowerTitle = rNewTitle.toAsciiLowerCase();
    }


    //= EnumerationResult

    enum class EnumerationResult
    {
        SUCCESS,    /// the enumeration was successful
        ERROR,      /// the enumeration was unsuccessful
    };


    //= FolderDescriptor

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

        explicit FolderDescriptor( const OUString& _rURL )
            :sURL( _rURL )
        {
        }
    };


    //= IEnumerationResultHandler

    class IEnumerationResultHandler
    {
    public:
        virtual void        enumerationDone( EnumerationResult _eResult ) = 0;

    protected:
        ~IEnumerationResultHandler() {}
    };


    //= FileViewContentEnumerator

    class FileViewContentEnumerator: public salhelper::Thread
    {
    public:
        typedef ::std::vector< std::unique_ptr<SortingData_Impl> >  ContentData;

    private:
        ContentData&                    m_rContent;
        ::osl::Mutex&                   m_rContentMutex;

        mutable ::osl::Mutex            m_aMutex;

        FolderDescriptor                m_aFolder;
        css::uno::Reference< css::ucb::XCommandEnvironment >
                                        m_xCommandEnv;
        IEnumerationResultHandler*      m_pResultHandler;
        bool                            m_bCancelled;

        css::uno::Sequence< OUString > m_rBlackList;

        bool URLOnBlackList ( const OUString& sRealURL );

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
            const css::uno::Reference< css::ucb::XCommandEnvironment >& _rxCommandEnv,
            ContentData& _rContentToFill,
            ::osl::Mutex& _rContentMutex
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
                    const css::uno::Sequence< OUString >& rBlackList
                );

        /** cancels the running operation.

            Note that "cancel" may mean that the operation is running, but its result
            is simply disregarded later on.
        */
        void    cancel();

    protected:
        virtual ~FileViewContentEnumerator() override;

    private:
        EnumerationResult enumerateFolderContent();

        // Thread overridables
        virtual void execute() override;

    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
