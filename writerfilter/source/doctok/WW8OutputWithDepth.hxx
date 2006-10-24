#ifndef INCLUDED_WW8_OUTPUT_WITH_DEPTH
#define INCLUDED_WW8_OUTPUT_WITH_DEPTH

#ifndef INCLUDED_OUTPUT_WITH_DEPTH
#include <doctok/OutputWithDepth.hxx>
#endif

#include <string>

using namespace ::std;

namespace doctok
{
class WW8OutputWithDepth : public OutputWithDepth<string>
{
protected:
    void output(const string & output) const;
public:
    WW8OutputWithDepth();
    virtual ~WW8OutputWithDepth() { finalize(); }
};

}

#endif // INCLUDED_WW8_OUTPUT_WITH_DEPTH
