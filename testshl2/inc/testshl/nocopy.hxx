#ifndef NOCOPY_HXX
#define NOCOPY_HXX

// prevent illegal copies
class NOCOPY
{
    NOCOPY(NOCOPY const&);
    NOCOPY& operator=(NOCOPY const&);
public:
    NOCOPY(){}
};

#endif
