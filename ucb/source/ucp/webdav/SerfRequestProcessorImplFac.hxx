/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef INCLUDED_SERFREQUESTPROCESSORIMPLFAC_HXX
#define INCLUDED_SERFREQUESTPROCESSORIMPLFAC_HXX

#include <vector>
#include <rtl/ustring.hxx>
#include "DAVTypes.hxx"
#include "DAVResource.hxx"

#include "SerfTypes.hxx"
#include "SerfRequestProcessorImpl.hxx"
#include "SerfInputStream.hxx"
#include <com/sun/star/io/XOutputStream.hpp>

namespace http_dav_ucp
{
    SerfRequestProcessorImpl* createPropFindReqProcImpl( const char* inPath,
                                                         const DAVRequestHeaders& inRequestHeaders,
                                                         const Depth inDepth,
                                                         const std::vector< ::rtl::OUString > & inPropNames,
                                                         std::vector< DAVResource > & ioResources );

    SerfRequestProcessorImpl* createPropFindReqProcImpl( const char* inPath,
                                                         const DAVRequestHeaders& inRequestHeaders,
                                                         const Depth inDepth,
                                                         std::vector< DAVResourceInfo > & ioResInfo );

    SerfRequestProcessorImpl* createPropPatchReqProcImpl( const char* inPath,
                                                          const DAVRequestHeaders& inRequestHeaders,
                                                          const std::vector< ProppatchValue > & inProperties,
                                                          const char* inLockToken );

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm );

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm,
                                                    const std::vector< ::rtl::OUString > & inHeaderNames,
                                                    DAVResource& ioResource );

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm );

    SerfRequestProcessorImpl* createGetReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm,
                                                    const std::vector< ::rtl::OUString > & inHeaderNames,
                                                    DAVResource& ioResource );

    SerfRequestProcessorImpl* createHeadReqProcImpl( const char* inPath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const std::vector< ::rtl::OUString > & inHeaderNames,
                                                     DAVResource& ioResource );

    SerfRequestProcessorImpl* createPutReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const char* inData,
                                                    const char* inLockToken,
                                                    apr_size_t inDataLen );

    SerfRequestProcessorImpl* createPostReqProcImpl( const char* inPath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inData,
                                                     apr_size_t inDataLen,
                                                     const char* inLockToken,
                                                     const char* inContentType,
                                                     const char* inReferer,
                                                     const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm );

    SerfRequestProcessorImpl* createPostReqProcImpl( const char* inPath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inData,
                                                     apr_size_t inDataLen,
                                                     const char* inLockToken,
                                                     const char* inContentType,
                                                     const char* inReferer,
                                                     const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm );

    SerfRequestProcessorImpl* createDeleteReqProcImpl( const char* inPath,
                                                       const DAVRequestHeaders& inRequestHeaders,
                                                       const char * inLockToken );

    SerfRequestProcessorImpl* createMkColReqProcImpl( const char* inPath,
                                                      const DAVRequestHeaders& inRequestHeaders,
                                                      const char * inLockToken );

    SerfRequestProcessorImpl* createCopyReqProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inDestinationPath,
                                                     const bool inOverwrite,
                                                     const char* inLockToken );

    SerfRequestProcessorImpl* createMoveReqProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const char* inDestinationPath,
                                                     const bool inOverwrite,
                                                     const char* inLockToken );

    SerfRequestProcessorImpl* createLockReqProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const ucb::Lock& inLock,
                                                     const char* inTimeout,
                                                     DAVPropertyValue & outLock );

    SerfRequestProcessorImpl* createLockRefreshProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const ucb::Lock& inLock,
                                                     const char* inToken,
                                                     const char* inTimeout,
                                                     DAVPropertyValue & outLock );

    SerfRequestProcessorImpl* createUnlockProcImpl( const char* inSourcePath,
                                                     const DAVRequestHeaders& inRequestHeaders,
                                                     const ucb::Lock& inLock,
                                                     const char* inToken );

} // namespace http_dav_ucp

#endif // INCLUDED_SERFREQUESTPROCESSORIMPLFAC_HXX
