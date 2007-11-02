/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_oldidldocu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:59:59 $
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

#ifndef ARY_DOC_D_OLDIDLDOCU_HXX
#define ARY_DOC_D_OLDIDLDOCU_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>
// USED SERVICES
#include <ary_i/ci_text2.hxx>




namespace ary
{
namespace inf
{
    class AtTag2;
    class DocuToken;
    class DocuTex2;
}
namespace doc
{
    using ::ary::inf::AtTag2;
    using ::ary::inf::DocuToken;
    using ::ary::inf::DocuTex2;



/** Wrapper for the old idl documentation format.
*/
class OldIdlDocu : public Node
{
  public:
                        OldIdlDocu();
                        ~OldIdlDocu();

    void                AddToken2Short(
                            DYN DocuToken &     let_drToken )
                                                { aShort.AddToken(let_drToken); }
    void                AddToken2Description(
                            DYN DocuToken &     let_drToken )
                                                { aDescription.AddToken(let_drToken); }
    void                AddToken2DeprecatedText(
                            DYN DocuToken &     let_drToken );
    void                AddAtTag(
                            DYN AtTag2 &        let_drAtTag )
                                                { aTags.push_back(&let_drAtTag); }
    void                SetPublished()          { bIsPublished = true; }
    void                SetDeprecated()         { bIsDeprecated = true; }
    void                SetOptional()           { bIsOptional = true; }
    void                SetExternShort(
                            const DocuTex2 &    i_pExternShort )
                                                { pExternShort = &i_pExternShort; }

    const DocuTex2 &    Short() const           { return pExternShort != 0 ? *pExternShort : aShort; }
    const DocuTex2 &    Description() const     { return aDescription; }
    const DocuTex2 &    DeprecatedText() const  { return aDeprecatedText; }
    const std::vector< AtTag2* > &
                        Tags() const            { return aTags; }
    bool                IsPublished() const     { return bIsPublished; }
    bool                IsDeprecated() const    { return bIsDeprecated; }
    bool                IsOptional() const      { return bIsOptional; }

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    DocuTex2            aShort;
    DocuTex2            aDescription;
    DocuTex2            aDeprecatedText;
    std::vector< AtTag2* >
                        aTags;
    const DocuTex2 *    pExternShort;
    bool                bIsPublished;
    bool                bIsDeprecated;
    bool                bIsOptional;
};




}   // namespace doc
}   // namespace ary
#endif
