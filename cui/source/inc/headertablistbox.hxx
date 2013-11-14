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



#ifndef _HEADERTABLISTBOX_HXX
#define _HEADERTABLISTBOX_HXX

#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>


class _HeaderTabListBox : public Control
{
private:
    HeaderBar                   maHeaderBar;
    SvHeaderTabListBox          maListBox;
protected:
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar* );
    virtual long                Notify( NotifyEvent& rNEvt );
public:
                                _HeaderTabListBox( Window* pParent, const ResId& rId );
    virtual                     ~_HeaderTabListBox();

    inline SvHeaderTabListBox&  GetListBox( void );
    inline HeaderBar&           GetHeaderBar( void );

    void                        ConnectElements( void );
                                    // should be called after all manipulations on elements are done
                                    // calcs real sizes depending on sizes of this
    void                        Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 );    // same meaning as Windows::Show()
    void                        Enable( bool bEnable = true, bool bChild = true );  // same meaning as Windows::Enable()
};

inline SvHeaderTabListBox& _HeaderTabListBox::GetListBox( void )
{
    return maListBox;
}

inline HeaderBar& _HeaderTabListBox::GetHeaderBar( void )
{
    return maHeaderBar;
}


#endif
