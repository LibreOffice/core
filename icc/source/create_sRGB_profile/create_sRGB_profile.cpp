/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: create_sRGB_profile.cpp,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: beppec56 $ $Date: 2007-12-09 10:19:51 $

  Derived by beppec56@openoffice.org from various examples
  in SampleICC library, the original copyright retained.

  Copyright:  © see below
*/

/*
 * The ICC Software License, Version 0.1
 *
 *
 * Copyright (c) 2003-2006 The International Color Consortium. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        The International Color Consortium (www.color.org)"
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "ICC" and "The International Color Consortium" must
 *    not be used to imply that the ICC organization endorses or
 *    promotes products derived from this software without prior
 *    written permission. For written permission, please see
 *    <http://www.color.org/>.
 *
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNATIONAL COLOR CONSORTIUM OR
 * ITS CONTRIBUTING MEMBERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the The International Color Consortium.
 *
 *
 * Membership in the ICC is encouraged when this software is used for
 * commercial purposes.
 *
 *
 * For more information on The International Color Consortium, please
 * see <http://www.color.org/>.
 *
 *
 */

#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "IccUtil.h"
#include "IccProfile.h"

#include "Vetters.h"
#include "CAT.h"
#include "CLUT.h"

const char * const icc_file_name = "sRGB-IEC61966-2.1.icc";
const char * const hxx_file_name = "sRGB-IEC61966-2.1.hxx";
const char * const this_file_name_and_location =" * external/icc/source/create_sRGB_profile/create_sRGB_profile.cpp";

const char* const description = "sRGB IEC61966-2.1";
//const char* const devicemanufact = "IEC http://www.iec.ch"; not used, device manufactured by OOo seems funny...
const char* const devicemodel = "IEC 61966-2.1 Default RGB colour space - sRGB";
const char* const copyright = "The Contents of this file are made available subject to the terms of GNU Lesser General Public License Version 2.1";

// the creation date is fixed, corresponds to the last time this file has been changed
// NOTE: change this date values whenever the data inside the profile are changed.
const int  data_last_changed_year =     2007;
const int  data_last_changed_month =    8;
const int  data_last_day =              31;
const int  data_last_changed_hour =     14;
const int  data_last_changed_minute =   9;

// the following string array it's the standard OOo header format
const char * const TheHeader1[] =
{
    "/*************************************************************************",
    " *",
    " *  OpenOffice.org - a multi-platform office productivity suite",
    " *",
    " *  sRGB-IEC61966-2.1.hxx",
    " *",
    " *  creator: create_sRGB_profile",
    NULL
};

const char * const TheHeader2[] =
{
    " *",
    " *  The Contents of this file are made available subject to",
    " *  the terms of GNU Lesser General Public License Version 2.1.",
    " *",
    " *",
    " *    GNU Lesser General Public License Version 2.1",
    " *    =============================================",
    " *    Copyright 2005 by Sun Microsystems, Inc.",
    " *    901 San Antonio Road, Palo Alto, CA 94303, USA",
    " *",
    " *    This library is free software; you can redistribute it and/or",
    " *    modify it under the terms of the GNU Lesser General Public",
    " *    License version 2.1, as published by the Free Software Foundation.",
    " *",
    " *    This library is distributed in the hope that it will be useful,",
    " *    but WITHOUT ANY WARRANTY; without even the implied warranty of",
    " *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU",
    " *    Lesser General Public License for more details.",
    " *",
    " *    You should have received a copy of the GNU Lesser General Public",
    " *    License along with this library; if not, write to the Free Software",
    " *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,",
    " *    MA  02111-1307  USA",
    " *",
    " ************************************************************************/",
    "",
    "#ifndef INCLUDED_ICC_SRGB_IEC61966_2_1_H",
    "#define INCLUDED_ICC_SRGB_IEC61966_2_1_H",
    "",
    "/***********************************************************************",
    " * this file is automatically generated by file",
    this_file_name_and_location,
    " * modify that file if you need to change something",
    " ***********************************************************************/",
    NULL // last string, a null
};

const char * const TheTail[] =
{
    "#endif /* INCLUDED_ICC_SRGB_IEC61966_2_1_H */",
    NULL
};

icFloatNumber computeIECRGBtoXYZ( icFloatNumber indata )
{
    double retval = 0.0;
    if(indata < 0.04045)
        retval = indata/12.92;
    else // apply the other conversion
        retval = pow( (indata + 0.055)/1.055 , 2.4);

    return retval;
}

icFloatNumber computeIEC_XYZtoRGB( icFloatNumber indata )
{
    icFloatNumber retval = 0.0;
    if(indata < 0.0031308)
        retval = indata*12.92;
    else // apply the other conversion
        retval =  1.055*pow( indata , 1.0/2.4) - 0.055;

    //  cout << retval << endl;
    return retval;
}

void dumpTag(FILE *outfile, CIccProfile *pIcc, icTagSignature sig)
{
    CIccTag *pTag = pIcc->FindTag(sig);
    char buf[64];
    CIccInfo Fmt;

    std::string contents;

    if (pTag)
    {
        fprintf(outfile, "\nContents of %s tag (%s)\n", Fmt.GetTagSigName(sig), icGetSig(buf, sig));
        fprintf(outfile,"outfile, Type:   ");

        if (pTag->IsArrayType())  fprintf(outfile, "Array of ");

        fprintf(outfile, "%s\n", Fmt.GetTagTypeSigName(pTag->GetType()));
        pTag->Describe(contents);
        fwrite(contents.c_str(), contents.length(), 1, outfile);
    }
    else
        fprintf(outfile, "Tag (%s) not found in profile\n", icGetSig(buf, sig));
}

void dumpProfile(FILE *outfile, const char * profileName)
{
    CIccProfile *pIcc;
    std::string sReport;
    icValidateStatus nStatus;

    pIcc = OpenIccProfile(profileName);

    if (!pIcc)
        printf("Unable to open '%s'\n", profileName);
    else
    {
        icHeader *pHdr = &pIcc->m_Header;
        CIccInfo Fmt;
        char buf[64];

        fprintf(outfile,"Profile:          '%s'\n", profileName);
        if(Fmt.IsProfileIDCalculated(&pHdr->profileID))
            fprintf(outfile,"Profile ID:        %s\n", Fmt.GetProfileID(&pHdr->profileID));
        else
            fprintf(outfile,"Profile ID:       Profile ID not calculated.\n");
        fprintf(outfile,"Size:             %ld(0x%lx) bytes\n", pHdr->size, pHdr->size);

        fprintf(outfile,"\nHeader\n");
        fprintf(outfile,"------\n");
        fprintf(outfile,"Attributes:       %s\n", Fmt.GetDeviceAttrName(pHdr->attributes));
        fprintf(outfile,"Cmm:              %s\n", Fmt.GetCmmSigName((icCmmSignature)(pHdr->cmmId)));
        fprintf(outfile,"Creation Date:    %d/%d/%d  %02u:%02u:%02u\n",
                pHdr->date.month, pHdr->date.day, pHdr->date.year,
                pHdr->date.hours, pHdr->date.minutes, pHdr->date.seconds);
        fprintf(outfile,"Creator:          %s\n", icGetSig(buf, pHdr->creator));
        fprintf(outfile,"Data Color Space: %s\n", Fmt.GetColorSpaceSigName(pHdr->colorSpace));
        fprintf(outfile,"Flags             %s\n", Fmt.GetProfileFlagsName(pHdr->flags));
        fprintf(outfile,"PCS Color Space:  %s\n", Fmt.GetColorSpaceSigName(pHdr->pcs));
        fprintf(outfile,"Platform:         %s\n", Fmt.GetPlatformSigName(pHdr->platform));
        fprintf(outfile,"Rendering Intent: %s\n", Fmt.GetRenderingIntentName((icRenderingIntent)(pHdr->renderingIntent)));
        fprintf(outfile,"Type:             %s\n", Fmt.GetProfileClassSigName(pHdr->deviceClass));
        fprintf(outfile,"Version:          %s\n", Fmt.GetVersionName(pHdr->version));
        fprintf(outfile,"Illuminant:       X=%.4lf, Y=%.4lf, Z=%.4lf\n",
                icFtoD(pHdr->illuminant.X),
                icFtoD(pHdr->illuminant.Y),
                icFtoD(pHdr->illuminant.Z));

        fprintf(outfile,"\nProfile Tags\n");
        fprintf(outfile,"------------\n");

        fprintf(outfile,"%25s    ID    %8s\t%8s\n", "Tag", "Offset", "Size");
        fprintf(outfile,"%25s  ------  %8s\t%8s\n", "----", "------", "----");

        int n;
        TagEntryList::iterator i;

        for (n=0, i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++, n++)
        {
            fprintf(outfile,"%25s  %s  %8ld\t%8ld\n", Fmt.GetTagSigName(i->TagInfo.sig),
                    icGetSig(buf, i->TagInfo.sig, false),
                    i->TagInfo.offset, i->TagInfo.size);
        }

        for (n=0, i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++, n++)
            dumpTag(outfile, pIcc, i->TagInfo.sig);
    }
    delete pIcc;
}

int main(int argc, char* argv[])
{
    const char* myName = path_tail(argv[0]);

    try
    {
        int N = 1024; // number of points in LUTs

        const char* const out_file_pathname = icc_file_name;

        icFloatNumber measuredBlack[3];
        icFloatNumber measuredWhite[3];

        measuredBlack[0] = 0.0;
        measuredBlack[1] = 0.0;
        measuredBlack[2] = 0.0;

        measuredWhite[0] = 0.3127;
        measuredWhite[1] = 0.3290;
        measuredWhite[2] = 0.3583;

        icFloatNumber*   redTRC = new icFloatNumber[N];
        icFloatNumber* greenTRC = new icFloatNumber[N];
        icFloatNumber*  blueTRC = new icFloatNumber[N];
        int i;
        for (i = 0; i < N; ++i)
        {
            // apply conversion from RGB to XYZ, moving the RGB value linearly from 0 to 100%
            // 1024 steps are computed
            redTRC[i] = computeIECRGBtoXYZ( (icFloatNumber)i/(N-1));
            greenTRC[i] = redTRC[i];
            blueTRC[i] = redTRC[i];
        }

        CIccProfile profile;
        profile.InitHeader();

        profile.m_Header.date.year = data_last_changed_year;
        profile.m_Header.date.month = data_last_changed_month;
        profile.m_Header.date.day = data_last_day;
        profile.m_Header.date.hours = data_last_changed_hour;
        profile.m_Header.date.minutes = data_last_changed_minute;
        profile.m_Header.date.seconds = 0;

        profile.m_Header.deviceClass = icSigDisplayClass;
        profile.m_Header.colorSpace = icSigRgbData;
        profile.m_Header.pcs = icSigXYZData;
        profile.m_Header.platform = icSigUnkownPlatform;
        profile.m_Header.attributes = static_cast<icUInt64Number>(icReflective);
        profile.m_Header.renderingIntent = icPerceptual;

        profile.m_Header.cmmId = 0x6E6F6E65; /* 'none' */
        profile.m_Header.model = 0x73524742;//sRGB

        profile.m_Header.version=icVersionNumberV2_1;

        // Required tags for a three-component matrix-based display profile, as layed
        // out in the ICC spec [sections 8.2 and 8.4.3] are:
        //   profileDescriptionTag
        //   copyrightTag
        //   mediaWhitePointTag
        //   chromaticAdaptationTag
        //   redMatrixColumnTag
        //   greenMatrixColumnTag
        //   blueMatrixColumnTag
        //   redTRCTag
        //   greenTRCTag
        //   blueTRCTag

        // profileDescriptionTag
        CIccTagTextDescription* descriptionTag = new CIccTagTextDescription;
        descriptionTag->SetText(description);
        profile.AttachTag(icSigProfileDescriptionTag, descriptionTag);

        //device model tag
        CIccTagTextDescription* deviceModelTag = new CIccTagTextDescription;
        deviceModelTag->SetText("IEC 61966-2.1 Default RGB colour space - sRGB  ");
        profile.AttachTag( icSigDeviceModelDescTag, deviceModelTag);

        // copyrightTag
        CIccTagText* copyrightTag = new CIccTagText;
        copyrightTag->SetText(copyright);
        profile.AttachTag(icSigCopyrightTag, copyrightTag);

        //device signature
        CIccTagSignature* deviceSign = new CIccTagSignature;
        deviceSign->SetValue( icSigCRTDisplay );
        profile.AttachTag( icSigTechnologyTag, deviceSign );

        // For displays, and for projected imagery in cinema, we assume complete
        // adaptation on the part of the viewer, and we treat the medium white
        // as the illuminant.
        icFloatNumber illuminantY = measuredWhite[1];
        icFloatNumber normalizedIlluminant[3];
        for (i = 0; i < 3; ++i)
            normalizedIlluminant[i] = measuredWhite[i] / illuminantY;
        CAT* CATToD50 = new CAT(icD50XYZ, normalizedIlluminant);

        icFloatNumber flare[3] = { 0, 0, 0 }; // perhaps oversimplified but...

        // mediaWhitePointTag
        CIccTagXYZ* whitePointTag = new CIccTagXYZ;
        icFloatNumber adaptedMediaWhite[3];
        CLUT::measuredXYZToAdaptedXYZ(adaptedMediaWhite, measuredWhite,
                                      flare, illuminantY, CATToD50);
        (*whitePointTag)[0].X = icDtoF(adaptedMediaWhite[0]);
        (*whitePointTag)[0].Y = icDtoF(adaptedMediaWhite[1]);
        (*whitePointTag)[0].Z = icDtoF(adaptedMediaWhite[2]);
        profile.AttachTag(icSigMediaWhitePointTag, whitePointTag);

        // mediaBlackPointTag
        CIccTagXYZ* blackPointTag = new CIccTagXYZ;
        icFloatNumber adaptedMediaBlack[3];
        CLUT::measuredXYZToAdaptedXYZ(adaptedMediaBlack, measuredBlack,
                                      flare, illuminantY, CATToD50);
        (*blackPointTag)[0].X = icDtoF(adaptedMediaBlack[0]);
        (*blackPointTag)[0].Y = icDtoF(adaptedMediaBlack[1]);
        (*blackPointTag)[0].Z = icDtoF(adaptedMediaBlack[2]);
        profile.AttachTag(icSigMediaBlackPointTag, blackPointTag);
        // chromaticAdaptationTag
        CIccTagS15Fixed16* chromaticAdaptationTag = CATToD50->makeChromaticAdaptationTag();
        profile.AttachTag(icSigChromaticAdaptationTag, chromaticAdaptationTag);

        CIccTagXYZ* redMatrixColumnTag = new CIccTagXYZ;
        //values from raccomandation of ICC for sRGB, D50 referenced characterisation data
        (*redMatrixColumnTag)[0].X = icDtoF(0.4360);
        (*redMatrixColumnTag)[0].Y = icDtoF(0.2225);
        (*redMatrixColumnTag)[0].Z = icDtoF(0.0139);
        profile.AttachTag(icSigRedMatrixColumnTag, redMatrixColumnTag);

        CIccTagXYZ* greenMatrixColumnTag = new CIccTagXYZ;

        //values from raccomandation of ICC for sRGB, D50 referenced characterisation data
        (*greenMatrixColumnTag)[0].X = icDtoF(0.3851);
        (*greenMatrixColumnTag)[0].Y = icDtoF(0.7169);
        (*greenMatrixColumnTag)[0].Z = icDtoF(0.0971);
        profile.AttachTag(icSigGreenMatrixColumnTag, greenMatrixColumnTag);

        CIccTagXYZ* blueMatrixColumnTag = new CIccTagXYZ;

        //values from raccomandation of ICC for sRGB, D50 referenced characterisation data
        (*blueMatrixColumnTag)[0].X = icDtoF(0.1431);
        (*blueMatrixColumnTag)[0].Y = icDtoF(0.0606);
        (*blueMatrixColumnTag)[0].Z = icDtoF(0.7139);
        profile.AttachTag(icSigBlueMatrixColumnTag, blueMatrixColumnTag);

        CIccTagCurve* redTRCTag = new CIccTagCurve(N);
        if (N == 1)
            redTRCTag->SetGamma(redTRC[0]);
        else
            for (i = 0; i < N; ++i)
                (*redTRCTag)[i] = redTRC[i];
        profile.AttachTag(icSigRedTRCTag, redTRCTag);

        CIccTagCurve* greenTRCTag = new CIccTagCurve(N);
        if (N == 1)
            greenTRCTag->SetGamma(greenTRC[0]);
        else
            for (i = 0; i < N; ++i)
                (*greenTRCTag)[i] = greenTRC[i];
        profile.AttachTag(icSigGreenTRCTag, greenTRCTag);

        CIccTagCurve* blueTRCTag = new CIccTagCurve(N);
        if (N == 1)
            blueTRCTag->SetGamma(blueTRC[0]);
        else
            for (i = 0; i < N; ++i)
                (*blueTRCTag)[i] = blueTRC[i];
        profile.AttachTag(icSigBlueTRCTag, blueTRCTag);

        //Verify things
        string validationReport;
        icValidateStatus validationStatus = profile.Validate(validationReport);

        switch (validationStatus)
        {
        case icValidateOK:
            break;

        case icValidateWarning:
            clog << "Profile validation warning" << endl
                 << validationReport;
            break;

        case icValidateNonCompliant:
            clog << "Profile non compliancy" << endl
                 << validationReport;
            break;

        case icValidateCriticalError:
        default:
            clog << "Profile Error" << endl
                 << validationReport;
        }

        // Out it goes
        CIccFileIO out;
        out.Open(out_file_pathname, "w+");
        profile.Write(&out);
        out.Close();

        FILE *headerfile = fopen(hxx_file_name,"w");

        //print OpenOffice standard file header
        const char *the_string;

        int idx = 0;

        while((the_string = TheHeader1[idx++]) != NULL )
            fprintf(headerfile,"%s\n",the_string);
// print the creation date (today)
// print the date of sRGB ICC profile data
        fprintf(headerfile," *  the date of last change to sRGB ICC profile data is:\n *  %4d/%02d/%02d - %02d:%02d\n",
                data_last_changed_year, data_last_changed_month,
                data_last_day, data_last_changed_hour,data_last_changed_minute );

        idx = 0;

        while((the_string = TheHeader2[idx++]) != NULL )
            fprintf(headerfile,"%s\n",the_string);

        {
// spit out the data structure (an array of unsigned char)
            FILE *infile;

            int achar, number = 1;

            infile = fopen(out_file_pathname,"r");

            fseek(infile,0,SEEK_END);
            long int thesize= ftell(infile);
            fseek(infile,0,SEEK_SET);

            fprintf(headerfile,"\nsal_uInt8 nsRGB_ICC_profile[%d]=\n{\n    ",thesize);

            do
            {
                achar = fgetc(infile);
                if(achar == EOF)
                    break;
                fprintf(headerfile,"0x%02x",achar);
                if(number % 12 == 0)
                    fprintf(headerfile,",\n    ");
                else
                    fprintf(headerfile,", ");
                number++;
            } while(achar != EOF );
            fprintf(headerfile,"\n};\n\n");

            fclose(infile);
        }
        // append the file contents, in human readable form, as comment in the header
        // get the functions from iccDump

        fprintf(headerfile,"/*****************\n\n");

        fprintf(headerfile,"This ICC profile contains the following data:\n\n");

        dumpProfile(headerfile, out_file_pathname );

        fprintf(headerfile,"\n*****************/\n");
        //now append the tail
        idx = 0;
        while((the_string = TheTail[idx++]) != NULL )
            fprintf(headerfile,"%s\n",the_string);

        fclose(headerfile);

        return EXIT_SUCCESS;
    }
    catch (const exception& e)
    {
        cout << myName << ": error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}
