/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* -----------------------------------------------------------------------
   Copyright (c) 1998, 2008 Red Hat, Inc.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

// shamelessly copied from libffi src/arm/sysv.h
#ifndef _OSL_ARMARCH_H_
#define _OSL_ARMARCH_H_
#ifdef ARM

#define __ARM_ARCH__ 3

#if defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__)
# undef __ARM_ARCH__
# define __ARM_ARCH__ 4
#endif

#if defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) \
    || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__)
# undef __ARM_ARCH__
# define __ARM_ARCH__ 5
#endif

#if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) \
    || defined(__ARM_ARCH_6M__)
# undef __ARM_ARCH__
# define __ARM_ARCH__ 6
#endif

#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)
# undef __ARM_ARCH__
# define __ARM_ARCH__ 7
#endif

#endif
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
