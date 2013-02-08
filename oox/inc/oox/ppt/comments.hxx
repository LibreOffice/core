#ifndef OOX_PPT_COMMENTS_HXX
#define OOX_PPT_COMMENTS_HXX

#define ELEMENT_NOT_FOUND 0

using rtl::OUString;
#include <vector>
#include <boost/algorithm/string.hpp> //split function to tokenize for date time
//#include "strtk.hpp" //tokenize the text of comments

#include <com/sun/star/util/DateTime.hpp>


class commentAuthor
{
  private:
  ::rtl::OUString clrIdx;
  ::rtl::OUString id;
  ::rtl::OUString initials;
  ::rtl::OUString lastIdx;
  ::rtl::OUString name;

  public:
  void setclrIdx(::rtl::OUString _clrIdx)
  {
    clrIdx = _clrIdx;
  }
  void setid(::rtl::OUString _id)
  {
    id=_id;
  }
  void setinitials(::rtl::OUString _initials)
  {
    initials=_initials;
  }
  void setlastIdx(::rtl::OUString _lastIdx)
  {
    lastIdx=_lastIdx;
  }
  void setname(::rtl::OUString _name)
  {
    name=_name;
  }
  ::rtl::OUString getclrIdx()
  {
    return clrIdx;
  }
  ::rtl::OUString getid()
  {
    return id;
  }
  ::rtl::OUString getinitials()
  {
    return initials;
  }
  ::rtl::OUString getlastIdx()
  {
    return lastIdx;
  }
  ::rtl::OUString getname()
  {
    return name;
  }
};

class commentAuthorList
{
  public:
  std::vector<commentAuthor> cmAuthorLst;
  
  void setValues( commentAuthorList list)
  {
    std::vector<commentAuthor>::iterator it;
    for(it=list.cmAuthorLst.begin();it!=list.cmAuthorLst.end();it++)
    {
      cmAuthorLst.push_back(commentAuthor());
      cmAuthorLst.back().setclrIdx(it->getclrIdx());
      cmAuthorLst.back().setid(it->getid());
      cmAuthorLst.back().setinitials(it->getinitials());
      cmAuthorLst.back().setlastIdx(it->getlastIdx());
      cmAuthorLst.back().setname(it->getname());
    }
  }
};

class comment
{
  private:
  ::rtl::OUString authorId;
  ::rtl::OUString dt;
  ::rtl::OUString idx;
  ::rtl::OUString x;
  ::rtl::OUString y;
  ::rtl::OUString text;
  ::com::sun::star::util::DateTime aDateTime;

  public:
  void setAuthorId(::rtl::OUString _aId)
  {
    authorId = _aId;
  }
  void setdt(::rtl::OUString _dt)
  {
    dt=_dt;
    setDateTime(_dt);
  }
  void setidx(::rtl::OUString _idx)
  {
    idx=_idx;
  }
  void set_X(::rtl::OUString _x)
  {
    x=_x;
  }
  void set_Y(::rtl::OUString _y)
  {
    y=_y;
  }
  void set_text(std::string _text)
  {
    text = rtl::OUString::createFromAscii (  _text.c_str() );
  }

  private:
  //DateTime is saved as : 2013-01-10T15:53:26.000
  void setDateTime (::rtl::OUString datetime)
  {
    std::string _datetime = rtl::OUStringToOString(datetime, RTL_TEXTENCODING_UTF8).getStr();
    std::vector<std::string> _dt;
    boost::split( _dt, _datetime, boost::is_any_of( "-:T" ) );
    aDateTime.Year = atoi(_dt.at(0).c_str());
    aDateTime.Month = atoi(_dt.at(1).c_str());
    aDateTime.Day = atoi(_dt.at(2).c_str());
    aDateTime.Hours = atoi(_dt.at(3).c_str());
    aDateTime.Minutes = atoi(_dt.at(4).c_str());
    aDateTime.HundredthSeconds = atoi(_dt.at(5).c_str());
    std::vector<std::string>::iterator i;
    
  }
  public:
  ::rtl::OUString getAuthorId()
  {
    return authorId;
  }
  ::rtl::OUString getdt()
  {
    return dt;
  }
  ::rtl::OUString getidx()
  {
    return idx;
  }
  ::rtl::OUString get_X()
  {
    return x;
  }
  ::rtl::OUString get_Y()
  {
    return y;
  }
  ::rtl::OUString get_text()
  {
    return text;
  }

  ::com::sun::star::util::DateTime getDateTime()
  {
    return aDateTime;
  }
  int get_int_X()
  {
    std::string temp = rtl::OUStringToOString(get_X(), RTL_TEXTENCODING_UTF8).getStr();
    return atoi(temp.c_str());
  }
  int get_int_Y()
  {
    std::string temp = rtl::OUStringToOString(get_Y(), RTL_TEXTENCODING_UTF8).getStr();
    return atoi(temp.c_str());
  }
  commentAuthor getAuthor ( commentAuthorList list )
  {
    std::string temp = rtl::OUStringToOString(authorId, RTL_TEXTENCODING_UTF8).getStr();
    int aId = atoi(temp.c_str());
    std::vector<commentAuthor>::iterator it;
    for(it = list.cmAuthorLst.begin(); it != list.cmAuthorLst.end(); it++)
    {
      temp = rtl::OUStringToOString(it->getid(), RTL_TEXTENCODING_UTF8).getStr();
      int list_aId = atoi(temp.c_str());
      if(list_aId == aId)
      return *(it);
    }
    throw ELEMENT_NOT_FOUND;
  }
};

class commentList
{
  public:
  std::vector<comment> cmLst;
  
  int getSize ()
  {
    return (int)cmLst.size();
  }
  comment getCommentAtIndex (int index)
  {
    if(index < (int)cmLst.size() && index >= 0)
      return cmLst.at(index);
    else
      throw ELEMENT_NOT_FOUND;
  }
};

#endif
