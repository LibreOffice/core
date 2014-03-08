#include "scCurrentCellPos.hxx"
#include <address.hxx>
scCurrentCellPos::scCurrentCellPos()
{
//idem
}
ScAddress scCurrentCellPos:: getCellAddr()
{
	return scCurrentCellPos::CurrentCellPos;
}
void scCurrentCellPos:: setCellAddr(ScAddress cAddr)
{
	scCurrentCellPos::CurrentCellPos=cAddr;
}
scCurrentCellPos::~scCurrentCellPos()
{
//idem
}
ScAddress scCurrentCellPos::CurrentCellPos;
