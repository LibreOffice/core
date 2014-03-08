#ifndef SCCURRENTCELLPOS_H
#define SCCURRENTCELLPOS_H
#include <address.hxx>

class scCurrentCellPos
{
    public:
       scCurrentCellPos();
        ScAddress getCellAddr();
        void setCellAddr(ScAddress);
        virtual ~scCurrentCellPos();
    protected:
    private:
        static ScAddress CurrentCellPos;
};

#endif // SCCURRENTCELLPOS_H
