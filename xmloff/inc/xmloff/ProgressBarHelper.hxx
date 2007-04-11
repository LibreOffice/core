/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProgressBarHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:20:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#define _XMLOFF_PROGRESSBARHELPER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#define XML_PROGRESSRANGE   "ProgressRange"
#define XML_PROGRESSMAX     "ProgressMax"
#define XML_PROGRESSCURRENT "ProgressCurrent"
#define XML_PROGRESSREPEAT  "ProgressRepeat"

class XMLOFF_DLLPUBLIC ProgressBarHelper
{
            ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator >   xStatusIndicator;
            sal_Int32                                                                       nRange;
            sal_Int32                                                                       nReference;
            sal_Int32                                                                       nValue;
            double                                                                          fOldPercent;
            sal_Bool                                                                        bStrict;
            // #96469#; if the value goes over the Range the progressbar starts again
            sal_Bool                                                                        bRepeat;

#ifndef PRODUCT
            sal_Bool                                                                        bFailure;
#endif
public:
            ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xStatusIndicator,
                                const sal_Bool bStrict);
            ~ProgressBarHelper();

            void SetText(::rtl::OUString& rText) { if (xStatusIndicator.is()) xStatusIndicator->setText(rText); }
            void SetRange(sal_Int32 nVal) { nRange = nVal; }
            void SetReference(sal_Int32 nVal) { nReference = nVal; }
            void SetValue(sal_Int32 nValue);
            void SetRepeat(sal_Bool bValue) { bRepeat = bValue; }
            inline void Increment(sal_Int32 nInc = 1) { SetValue( nValue+nInc ); }
            void End() { if (xStatusIndicator.is()) xStatusIndicator->end(); }

            // set the new reference and returns the new value which gives the
            // Progress Bar the sam position as before
            sal_Int32 ChangeReference(sal_Int32 nNewReference);

            sal_Int32 GetReference() { return nReference; }
            sal_Int32 GetValue() { return nValue; }
            sal_Bool GetRepeat() { return bRepeat; }
};

#endif

