@echo off
set BASE=%~pd0%.
cd %BASE%
ant routine.test >"%BASE%\..\testspace\test.log" 2>&1