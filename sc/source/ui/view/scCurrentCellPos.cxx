#include "scCurrentCellPos.hxx"
#include <address.hxx>
scCurrentCellPos::scCurrentCellPos()
{
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
}
ScAddress scCurrentCellPos::CurrentCellPos;
