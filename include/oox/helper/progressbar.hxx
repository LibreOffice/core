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

#ifndef INCLUDED_OOX_HELPER_PROGRESSBAR_HXX
#define INCLUDED_OOX_HELPER_PROGRESSBAR_HXX

#include <memory>
#include <com/sun/star/uno/Reference.hxx>
#include <oox/dllapi.h>


namespace com { namespace sun { namespace star {
    namespace task { class XStatusIndicator; }
} } }

namespace oox {

// Interfaces =================================================================

/** Interface for progress bar classes.
 */
class OOX_DLLPUBLIC IProgressBar
{
public:
    virtual             ~IProgressBar();

    /** Returns the current position of the progress bar.

        @return  Position of the progress bar, in the range from 0.0 (beginning
        of the progress bar) to 1.0 (end of the progress bar) inclusive.
     */
    virtual double      getPosition() const = 0;

    /** Sets the current position of the progress bar.

        @param fPosition  New position of the progress bar, in the range from
        0.0 (beginning of the progress bar) to 1.0 (end of the progress bar)
        inclusive.
     */
    virtual void        setPosition( double fPosition ) = 0;
};




class ISegmentProgressBar;
typedef std::shared_ptr< ISegmentProgressBar > ISegmentProgressBarRef;

/** Interface for a segment in a progress bar, that is able to create sub
    segments from itself.
 */
class OOX_DLLPUBLIC ISegmentProgressBar : public IProgressBar
{
public:
    virtual             ~ISegmentProgressBar();

    /** Returns the length that is still free for creating sub segments. */
    virtual double      getFreeLength() const = 0;

    /** Adds a new segment with the specified length. */
    virtual ISegmentProgressBarRef createSegment( double fLength ) = 0;
};




/** A simple progress bar.
 */
class OOX_DLLPUBLIC ProgressBar : public IProgressBar
{
public:
    explicit            ProgressBar(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& rxIndicator,
                            const OUString& rText );

    virtual             ~ProgressBar();

    /** Returns the current position of the progress bar. */
    virtual double      getPosition() const override;
    /** Sets the current position of the progress bar. */
    virtual void        setPosition( double fPosition ) override;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >
                        mxIndicator;
    double              mfPosition;
};



/** A progress bar containing several independent segments.
 */
class OOX_DLLPUBLIC SegmentProgressBar : public ISegmentProgressBar
{
public:
    explicit            SegmentProgressBar(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& rxIndicator,
                            const OUString& rText );

    /** Returns the current position of the progress bar segment. */
    virtual double      getPosition() const override;
    /** Sets the current position of the progress bar segment. */
    virtual void        setPosition( double fPosition ) override;

    /** Returns the length that is still free for creating sub segments. */
    virtual double      getFreeLength() const override;
    /** Adds a new segment with the specified length. */
    virtual ISegmentProgressBarRef createSegment( double fLength ) override;

private:
    ProgressBar         maProgress;
    double              mfFreeStart;
};



} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
