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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SMARTTAG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SMARTTAG_HXX

#include <helper/simplereferencecomponent.hxx>
#include <rtl/ref.hxx>
#include <set>
#include <svx/svdhdl.hxx>
#include <svx/svdview.hxx>

class KeyEvent;
class MouseEvent;

namespace sd {

class View;
class SmartHdl;

/** a smart tag represents a visual user interface element on the documents edit view
    that is not part of the document. It uses derivations from SmartHdl for its visuals.
    A SmartTag adds himself to the given view if created. It removes himself if it
    is disposed before the view is disposed.

    Derive from this class to implement your own smart tag.
*/
class SmartTag : public SimpleReferenceComponent
{
    friend class SmartTagSet;

public:
    explicit SmartTag( ::sd::View& rView );
    virtual ~SmartTag() override;

    /** returns true if the SmartTag consumes this event. */
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& );

    /** returns true if the SmartTag consumes this event. */
    virtual bool KeyInput( const KeyEvent& rKEvt );

    /** returns true if the SmartTag consumes this event. */
    virtual bool RequestHelp( const HelpEvent& rHEvt );

    /** returns true if the SmartTag consumes this event. */
    virtual bool Command( const CommandEvent& rCEvt );

    /** returns true if this smart tag is currently selected */
    bool isSelected() const { return mbSelected;}

    ::sd::View& getView() const { return mrView; }

protected:
    virtual sal_Int32 GetMarkablePointCount() const;
    virtual sal_Int32 GetMarkedPointCount() const;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark);
    virtual void CheckPossibilities();
    virtual bool MarkPoints(const ::tools::Rectangle* pRect, bool bUnmark);

    virtual void addCustomHandles( SdrHdlList& rHandlerList );
    virtual void select();
    virtual void deselect();
    virtual bool getContext( SdrViewContext& rContext );

    virtual void disposing() override;

    ::sd::View& mrView;
    bool mbSelected;

private:
    SmartTag( const SmartTag& ) = delete;
    SmartTag& operator=( const SmartTag& ) = delete;
};

typedef rtl::Reference< SmartTag > SmartTagReference;

/** class to administrate the available smart tags for a single view. */
class SmartTagSet
{
    friend class SmartTag;
public:
    explicit SmartTagSet( ::sd::View& rView );
    ~SmartTagSet();

    /** selects the given smart tag and updates all handles */
    void select( const SmartTagReference& xTag );

    /** deselects the current selected smart tag and updates all handles */
    void deselect();

    /** returns the currently selected tag or an empty reference. */
    const SmartTagReference& getSelected() const { return mxSelectedTag; }

    /** returns true if a SmartTag consumes this event. */
    bool MouseButtonDown( const MouseEvent& );

    /** returns true if a SmartTag consumes this event. */
    bool KeyInput( const KeyEvent& rKEvt );

    /** returns true if a SmartTag consumes this event. */
    bool RequestHelp( const HelpEvent& rHEvt );

    /** returns true if a SmartTag consumes this event. */
    bool Command( const CommandEvent& rCEvt );

    /** disposes all smart tags and clears the set */
    void Dispose();

    /** adds the handles from all smart tags to the given list */
    void addCustomHandles( SdrHdlList& rHandlerList );

    /** returns true if the currently selected smart tag has
        a special context, returned in rContext. */
    bool getContext( SdrViewContext& rContext ) const;

    // support point editing
    bool HasMarkablePoints() const;
    sal_uLong GetMarkablePointCount() const;
    bool HasMarkedPoints() const;
    sal_uLong GetMarkedPointCount() const;
    static bool IsPointMarkable(const SdrHdl& rHdl);
    bool MarkPoint(SdrHdl& rHdl, bool bUnmark);
    bool MarkPoints(const ::tools::Rectangle* pRect, bool bUnmark);

    void CheckPossibilities();

private:
    SmartTagSet( const SmartTagSet& ) = delete;
    SmartTagSet& operator=( const SmartTagSet& ) = delete;

    /** adds a new smart tag to this set */
    void add( const SmartTagReference& xTag );

    /** removes the given smart tag from this set */
    void remove( const SmartTagReference& xTag );

     std::set< SmartTagReference > maSet;

    ::sd::View& mrView;
    SmartTagReference mxSelectedTag;
    SmartTagReference mxMouseOverTag;
};

/** a derivation from this handle is the visual representation for a smart tag.
    One smart tag can have more than one handle.
*/
class SmartHdl : public SdrHdl
{
public:
    SmartHdl( const SmartTagReference& xTag, SdrObject* pObject, const Point& rPnt, SdrHdlKind eNewKind );
    SmartHdl( const SmartTagReference& xTag, const Point& rPnt, SdrHdlKind eNewKind );

    const SmartTagReference& getTag() const { return mxSmartTag; }

    virtual bool isMarkable() const;
private:
    SmartTagReference const mxSmartTag;
};

} // end of namespace sd

#endif // INCLUDED_SD_SOURCE_UI_INC_SMARTTAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
