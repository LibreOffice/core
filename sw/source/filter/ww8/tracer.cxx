/*************************************************************************
 *
 *  $RCSfile: tracer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:21:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>         //INetURLObject
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>         //SfxMedium
#endif
#ifndef _MS_FILTERTRACER_HXX
#include <svx/msfiltertracer.hxx>   //MSFilterTracer
#endif
#ifndef SW_TRACER_HXX
#include "tracer.hxx"               //sw::log::Tracer
#endif

using rtl::OUString;
#define CAU(X) RTL_CONSTASCII_USTRINGPARAM(X)
#define C2O(X) OUString(CAU(X))

namespace sw
{
    namespace log
    {
        Tracer::Tracer(const SfxMedium &rMed)
            : mpTrace(0)
        {
            using namespace com::sun::star::uno;
            using namespace ::com::sun::star::beans;
            Sequence<PropertyValue> aConfigData(1);
            PropertyValue aPropValue;
            aPropValue.Value <<=
                OUString(rMed.GetURLObject().GetMainURL(
                    INetURLObject::NO_DECODE));
            aPropValue.Name = C2O("DocumentURL");
            aConfigData[0] = aPropValue;
            OUString aTraceConfigPath(CAU("Office.Tracing/Import/Word"));
            if (mpTrace = new MSFilterTracer(aTraceConfigPath, &aConfigData))
                mpTrace->StartTracing();
        }

        Tracer::~Tracer()
        {
            if (mpTrace)
            {
                mpTrace->EndTracing();
                delete mpTrace;
            }
        }

        void Tracer::Log(Problem eProblem)
        {
            if (!mpTrace)
                return;

            OUString sID(CAU("sw"));
            sID += rtl::OUString::valueOf(static_cast<sal_Int32>(eProblem));
            switch (eProblem)
            {
                case ePrinterMetrics:
                    mpTrace->Trace(sID,
                        C2O("Word<->Writer \'Don't Use PrinterMetrics\' not"
                            " exactly equivalent"));
                    break;
                case eExtraLeading:
                    mpTrace->Trace(sID,
                        C2O("\'Use Extra Leading\' not supported"));
                    break;
                case eTabStopDistance:
                    mpTrace->Trace(sID,
                        C2O("Minimum tab distance larger in Writer"));
                    break;
                case eDontUseHTMLAutoSpacing:
                    mpTrace->Trace(sID,
                        C2O("\'Don't Use HTML AutoSpacing\' affects more than"
                            " what is currently implemented or understood"));
                    break;
                case eAutoWidthFrame:
                    mpTrace->Trace(sID,
                        C2O("No core support for autowidth frames"));
                    break;
                default:
                    mpTrace->Trace(sID, C2O("Unknown trace property"));
                    break;
            }
        }

        Context::Context(const Tracer &rTracer, Environment eContext)
            : mpTrace(rTracer.GetTrace())
        {
            switch (eContext)
            {
                case eDocumentProperties:
                    msContext = C2O("Global");
                    msDetails = C2O("Document Properties");
                    break;
                case eMainText:
                    msContext = C2O("Text");
                    msDetails = C2O("MainDocument");
                    break;
                case eSubDoc:
                    msContext = C2O("Text");
                    msDetails = C2O("Unknown Subdocument");
                    break;
                default:
                    msContext = C2O("UNKNOWN");
                    msDetails = C2O("UNKNOWN");
                    break;
            }

            if (mpTrace)
                mpTrace->AddAttribute(msContext, msDetails);
        }

        Context::~Context()
        {
            if (mpTrace)
                mpTrace->RemoveAttribute(msContext);
        }
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
