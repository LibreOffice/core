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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <SerfRequestProcessorImplFac.hxx>
#include <SerfPropFindReqProcImpl.hxx>
#include <SerfPropPatchReqProcImpl.hxx>
#include <SerfGetReqProcImpl.hxx>
#include <SerfHeadReqProcImpl.hxx>
#include <SerfPutReqProcImpl.hxx>
#include <SerfPostReqProcImpl.hxx>
#include <SerfDeleteReqProcImpl.hxx>
#include <SerfMkColReqProcImpl.hxx>
#include <SerfCopyReqProcImpl.hxx>
#include <SerfMoveReqProcImpl.hxx>

namespace http_dav_ucp
{
    SerfRequestProcessorImpl* createPropFindReqProcImpl( const char* inPath,
                                                         const DAVRequestHeaders& inRequestHeaders,
                                                         const Depth inDepth,
                                                         const std::vector< ::rtl::OUString > & inPropNames,
                                                         std::vector< DAVResource > & ioResources )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfPropFindReqProcImpl( inPath,
                                                                              inRequestHeaders,
                                                                              inDepth,
                                                                              inPropNames,
                                                                              ioResources );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createPropFindReqProcImpl( const char* inPath,
                                                         const DAVRequestHeaders& inRequestHeaders,
                                                         const Depth inDepth,
                                                         std::vector< DAVResourceInfo > & ioResInfo )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfPropFindReqProcImpl( inPath,
                                                                              inRequestHeaders,
                                                                              inDepth,
                                                                              ioResInfo );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createPropPatchReqProcImpl( const char* inPath,
                                                          const DAVRequestHeaders& inRequestHeaders,
                                                          const std::vector< ProppatchValue > & inProperties )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfPropPatchReqProcImpl( inPath,
                                                                               inRequestHeaders,
                                                                               inProperties );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfGetReqProcImpl( inPath,
                                                                         inRequestHeaders,
                                                                         xioInStrm );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm,
                                                    const std::vector< ::rtl::OUString > & inHeaderNames,
                                                    DAVResource& ioResource )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfGetReqProcImpl( inPath,
                                                                         inRequestHeaders,
                                                                         xioInStrm,
                                                                         inHeaderNames,
                                                                         ioResource );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfGetReqProcImpl( inPath,
                                                                         inRequestHeaders,
                                                                         xioOutStrm );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference<com::sun::star::io::XOutputStream >& xioOutStrm,
                                                    const std::vector< ::rtl::OUString > & inHeaderNames,
                                                    DAVResource& ioResource )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfGetReqProcImpl( inPath,
                                                                         inRequestHeaders,
                                                                         xioOutStrm,
                                                                         inHeaderNames,
                                                                         ioResource );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createHeadReqProcImpl( const char* inPath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const std::vector< ::rtl::OUString > & inHeaderNames,
                                                     DAVResource& ioResource )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfHeadReqProcImpl( inPath,
                                                                          inRequestHeaders,
                                                                          inHeaderNames,
                                                                          ioResource );
        return pReqProcImpl;
    }


    SerfRequestProcessorImpl* createPutReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const char* inData,
                                                    apr_size_t inDataLen )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfPutReqProcImpl( inPath,
                                                                         inRequestHeaders,
                                                                         inData,
                                                                         inDataLen );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createPostReqProcImpl( const char* inPath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inData,
                                                     apr_size_t inDataLen,
                                                     const char* inContentType,
                                                     const char* inReferer,
                                                     const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfPostReqProcImpl( inPath,
                                                                          inRequestHeaders,
                                                                          inData,
                                                                          inDataLen,
                                                                          inContentType,
                                                                          inReferer,
                                                                          xioInStrm );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createPostReqProcImpl( const char* inPath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inData,
                                                     apr_size_t inDataLen,
                                                     const char* inContentType,
                                                     const char* inReferer,
                                                     const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfPostReqProcImpl( inPath,
                                                                          inRequestHeaders,
                                                                          inData,
                                                                          inDataLen,
                                                                          inContentType,
                                                                          inReferer,
                                                                          xioOutStrm );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createDeleteReqProcImpl( const char* inPath,
                                                       const DAVRequestHeaders& inRequestHeaders )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfDeleteReqProcImpl( inPath,
                                                                            inRequestHeaders );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createMkColReqProcImpl( const char* inPath,
                                                      const DAVRequestHeaders& inRequestHeaders )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfMkColReqProcImpl( inPath,
                                                                           inRequestHeaders );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createCopyReqProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inDestinationPath,
                                                     const bool inOverwrite )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfCopyReqProcImpl( inSourcePath,
                                                                          inRequestHeaders,
                                                                          inDestinationPath,
                                                                          inOverwrite );
        return pReqProcImpl;
    }

    SerfRequestProcessorImpl* createMoveReqProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inDestinationPath,
                                                     const bool inOverwrite )
    {
        SerfRequestProcessorImpl* pReqProcImpl = new SerfMoveReqProcImpl( inSourcePath,
                                                                          inRequestHeaders,
                                                                          inDestinationPath,
                                                                          inOverwrite );
        return pReqProcImpl;
    }

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
