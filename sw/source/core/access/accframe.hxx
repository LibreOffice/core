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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRAME_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRAME_HXX

#include <swrect.hxx>

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <list>
#include <accfrmobj.hxx>

class SwAccessibleMap;
class SwFrame;
class SwViewShell;
namespace sw { namespace access {
    class SwAccessibleChild;
}}

// Any method of this class must be called with an acquired solar mutex!

class SwAccessibleFrame
{
    SwRect maVisArea;
    const SwFrame* mpFrame;
    const bool mbIsInPagePreview;

protected:
    // #i77106# - method needs to be called by new class <SwAccessibleTableColHeaders>
    static sal_Int32 GetChildCount( SwAccessibleMap& rAccMap,
                                    const SwRect& rVisArea,
                                    const SwFrame *pFrame,
                                    bool bInPagePreviewr );

// private:
    static sw::access::SwAccessibleChild GetChild( SwAccessibleMap& rAccMap,
                                                   const SwRect& rVisArea,
                                                   const SwFrame& rFrame,
                                                   sal_Int32& rPos,
                                                   bool bInPagePreview);

    static bool GetChildIndex( SwAccessibleMap& rAccMap,
                               const SwRect& rVisArea,
                               const SwFrame& rFrame,
                               const sw::access::SwAccessibleChild& rChild,
                               sal_Int32& rPos,
                               bool bInPagePreview );

    static sw::access::SwAccessibleChild GetChildAtPixel( const SwRect& rVisArea,
                                                          const SwFrame& rFrame,
                                                          const Point& rPos,
                                                          bool bInPagePreview,
                                                          SwAccessibleMap& rAccMap );

    static void GetChildren( SwAccessibleMap& rAccMap,
                             const SwRect& rVisArea,
                             const SwFrame& rFrame,
                             std::list< sw::access::SwAccessibleChild >& rChildren,
                             bool bInPagePreview );

    bool IsEditable( SwViewShell *pVSh ) const;

    bool IsOpaque( SwViewShell *pVSh ) const;

public:
    bool IsShowing( const SwAccessibleMap& rAccMap,
                        const sw::access::SwAccessibleChild& rFrameOrObj ) const;
    inline bool IsShowing( const SwRect& rFrame ) const;
    inline bool IsShowing( const SwAccessibleMap& rAccMap ) const;

protected:
    inline bool IsInPagePreview() const
    {
        return mbIsInPagePreview;
    }

    inline void ClearFrame()
    {
        mpFrame = nullptr;
    }

    SwAccessibleFrame( const SwRect& rVisArea,
                       const SwFrame *pFrame,
                       bool bIsPagePreview );
    virtual ~SwAccessibleFrame();

    // MT: Move to private area?
    bool m_bIsAccDocUse;
public:
    // Return the SwFrame this context is attached to.
    const SwFrame* GetFrame() const { return mpFrame; };

    static const SwFrame* GetParent( const sw::access::SwAccessibleChild& rFrameOrObj,
                                   bool bInPagePreview );

    sal_Int32 GetChildIndex( SwAccessibleMap& rAccMap,
                             const sw::access::SwAccessibleChild& rChild ) const;

protected:
    // Return the bounding box of the frame clipped to the vis area. If
    // no frame is specified, use this' frame.
    SwRect GetBounds( const SwAccessibleMap& rAccMap,
                      const SwFrame *pFrame = nullptr );

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    inline const SwFrame *GetParent() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrame's lowers
    sal_Int32 GetChildCount( SwAccessibleMap& rAccMap ) const;
    sw::access::SwAccessibleChild GetChild( SwAccessibleMap& rAccMap,
                                            sal_Int32 nPos ) const;
    sw::access::SwAccessibleChild GetChildAtPixel( const Point& rPos,
                                                   SwAccessibleMap& rAccMap ) const;
    void GetChildren( SwAccessibleMap& rAccMap,
                      std::list< sw::access::SwAccessibleChild >& rChildren ) const;

    inline void SetVisArea( const SwRect& rNewVisArea )
    {
        maVisArea = rNewVisArea;
    }

    inline const SwRect& GetVisArea() const
    {
        return maVisArea;
    }

    OUString GetFormattedPageNumber() const;
};

inline bool SwAccessibleFrame::IsShowing( const SwRect& rFrame ) const
{
    return !rFrame.IsEmpty() && rFrame.IsOver( maVisArea );
}

inline bool SwAccessibleFrame::IsShowing( const SwAccessibleMap& rAccMap ) const
{
    sw::access::SwAccessibleChild aFrameOrObj( GetFrame() );
    return IsShowing( rAccMap, aFrameOrObj );
}

inline const SwFrame *SwAccessibleFrame::GetParent() const
{
    sw::access::SwAccessibleChild aFrameOrObj( GetFrame() );
    return GetParent( aFrameOrObj, IsInPagePreview()  );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
