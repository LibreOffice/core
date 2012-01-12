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




#if defined (SOLARIS) || (FREEBSD)

#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

/* no frame.h on FreeBSD */
#if defined FREEBSD
struct frame {
    long    arg0[8];
    long    arg1[6];
    struct frame *fr_savfp;
    long    fr_savpc;
};
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined SOLARIS || FREEBSD */

#if defined (LINUX) && defined (SPARC)
#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

/* no frame.h on linux sparc */
struct frame {
    long    arg0[8];
    long    arg1[6];
    struct frame *fr_savfp;
    long    fr_savpc;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined LINUX && SPARC */

#if defined (MACOSX)

#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined MACOSX */
