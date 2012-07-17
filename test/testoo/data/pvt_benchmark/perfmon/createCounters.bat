logman stop pvt
logman delete counter pvt
logman create counter pvt -cf %2
logman update pvt -si 10 -f csv -o %1
logman start pvt
exit