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



#ifndef SVX_TABLE_CELLEDITSOURCE_HXX
#define SVX_TABLE_CELLEDITSOURCE_HXX

#include "cell.hxx"
#include "celltypes.hxx"
#include "editeng/unoedsrc.hxx"

class SvxTextForwarder;
class SdrObject;
class SdrModel;
class SdrView;
class Window;

namespace sdr { namespace table {

class CellEditSourceImpl;

class CellEditSource : public SvxEditSource, public SvxViewForwarder
{
public:
    CellEditSource( const CellRef& xCell );

    /** Since the views don't broadcast their dying, make sure that
        this object gets destroyed if the view becomes invalid

        The window is necessary, since our views can display on multiple windows
     */
    CellEditSource( const CellRef& xCell, SdrView& rView, const Window& rViewWindow );
    virtual ~CellEditSource();

    virtual SvxEditSource*          Clone() const;
    virtual SvxTextForwarder*       GetTextForwarder();
     virtual SvxViewForwarder*      GetViewForwarder();
     virtual SvxEditViewForwarder*  GetEditViewForwarder( sal_Bool bCreate = sal_False );
    virtual void                    UpdateData();

    virtual void addRange( SvxUnoTextRangeBase* pNewRange );
    virtual void removeRange( SvxUnoTextRangeBase* pOldRange );
    virtual const SvxUnoTextRangeBaseList& getRanges() const;

    virtual SfxBroadcaster&         GetBroadcaster() const;

    void lock();
    void unlock();

    // the SvxViewForwarder interface
    virtual sal_Bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point&, const MapMode& ) const;
    virtual Point       PixelToLogic( const Point&, const MapMode& ) const;

private:
    CellEditSource( CellEditSourceImpl* pImpl );

    CellEditSourceImpl* mpImpl;
};

} }

#endif
