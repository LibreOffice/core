# Pravila za automatsko ispravljanje, iznimke kraja rečenice te iznimke ispravljanja dvaju velikih početnih slova na početku riječi

Za LibreOfficeovu su značajku automatskog ispravljanja važne datoteke

* DocumentList.xml
* SentenceExceptList.xml
* WordExceptList.xml

Budući da se ovaj repozitorij ažurira češće negoli izvorni kôd LibreOfficea, upute se kako koristiti noviji popis uz stariju inačicu LibreOfficea nalaze u datoteci [umetanje-automatskih-ispravaka.md](https://github.com/krunose/libo-acor-hr/blob/master/umetanje-automatskih-ispravaka.md).

Službena je dokumentacija pomalo štura:

[https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras](https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras)


## DocumentList.xml
Datoteka sadrži popis pravila za automatsko ispravljanje riječi ili određenih kombinacija znakova s novom riječi ili novom kombinacijom znakova.

Tako će znakovi :^2: dati drugu potenciju, odnosno X:^2: daje X². [[Mogući ispravci](https://github.com/krunose/libo-acor-hr/blob/master/DocumentList.xml)]

U ovu je datoteku poželjno dodavati i one ispravke koji su **nedvojbeno** zatipci, ili pak kada je riječ o ispravljanju maloga i velikoga slova u dvojnim kraticama zbog točke: dr. sc. > dr. Sc. > dr. sc. (v. datoteku [SentenceExceptList.xml](https://github.com/krunose/libo-acorr-hr/blob/master/SentenceExceptList.xml)).

U se krnjī infinitiv ako iza njega slijedi nenaglašeni oblik pomoćnog glagola htjeti: tražit ću. Nije dobro u ovu datoteku unositi takve ispravke jer, kako se radi o čestoj pogrešci, netko ju možda želi napisati namjerno, kao primjer. Računalo bi svaki takav primjer ispravljalo bez ikakva upozorenja ili podcrtavanja. Slično tako i riječi 'avijon', 'idijot', 'kemia'... O takvim pogreškama treba brinuti računalna provjera pravopisa i gramatike jer podcrtava grešku, ali korisniku daje izbor želi li ostati pri svome ili ispraviti. Uvijek treba biti oprezan i izbjegavati situacije u kojima računalo izvršava radnje bez eksplicitne potvrde korisnika.

U engleskoj se inačici pravila za automatsko ispravljanje nalazi (ili se nalazilo) pravilo koje zatipak 'nwe' zamjenjuje sa 'new', što je se na prvi pogled čini dobrim rješenjem. Međutim, jedan je korisnik automatskog ispravljanja za engleski jezik trebao i **želio** napisati 'nwe'. LibreOffice je svaki 'nwe' zamijenio s 'new' bez ikakvog upozorenja i korisnik je primijetio pogrešku kada je već bilo kasno. Štetu nije moguće ispraviti automatskom zamjenom teksta (engl. find and replace) jer bi se zamjena provela u oba slučaja – gdje treba i gdje ne treba; jedino je rješenje pregledati cijeli tekst i ispraviti pogreške ručno.

U ovom bi slučaju podcrtavanje pomoću provjere pravopisa ili gramatike bilo bolje rješnje jer bi korisnik bio upozoren na potencijalnu pogrešku, ali sadržaj ne bi bio uništen neželjenim ispravljanjem.

U ovu datoteku **treba** dodati ono što je nedvojbeno zatipak i ono što bi nepotrebno opterećivalo rječnik za provjeru pravopis ili povećavalo broj pravila za provjeri gramatike, recimo primjere poput 'pokuša tću'. Nema potrebe da korisnik eksplicitno potvrđuje ovakve situacije. Ali i opet treba biti oprezan.

Nevolja je u tome što korisnik ne može znati što se sve nalazi na ovom popisu i onaj tko sastavlja popis nikada ne može predvidjeti sve moguće upotrebe i korištenja popisa pa treba biti oprezan oko dodavanja pravila. Bolje je izostaviti nego dodatni pogrešno.

---

Popis se zamjena koje su dodani mimo inicijalnih/standardnih nalazi u datoteci [dodano.txt](https://github.com/krunose/libo-acor-hr/blob/master/dodano-na-documentlist.txt).


## SentenceExceptList.xml
Točka označava kraj rečenice. LibreOffice automatski riječ iza točke ispravlja iz maloga početnoga slova u veliko. Međutim, nije svaka točka i znak kraja rečenice. Problem su kratice i skraćenice iza kojih se piše točka, a ne označavaju kraj rečenice. U običnom bi vezanom tekstu (članci, seminarski i diplomski radovi) trebalo izbjegavati pretjeranu upotrebu kratica.

Da bi se spriječilo automatsko ispravljanje malog slova u veliko iza kratica, na ovaj popis treba dodati kratice koje se ne mogu nalaziti na samome kraju rečenice, ili se tamo nalaze vrlo rijetko.

Jasnije će biti na primjeru 'Tako je 2016. god. odlučeno [...]'. Kada se kratica 'god.' ne bi nalazila na popisu iznimaka ispravljanja maloga slova u veliko, čim bi korisnik napisao 'odlučeno', LibreOffice bi automatski 'odlučeno' ispravio u 'Odlučeno': 'Tako je 2016. god. Odlučeno [...]'.

Uvrštavanjem kratice 'god.' na ovaj popis, LibreOffice ne uzima točku u kratici kao znak kraja rečenice već je ignorira.

Treba biti oprezan i ne uključivati kratice koje su izrazom jednake riječima koje doista i mogu stajati na kraju rečenice: 'čest.' od 'čestica' i 'čest' kao pridjev jer ta riječ može stajati na kraju rečenice.

Potrebno je vagati i provjeravati u jezičnim korpusima. Pitanje je javlja li se češće riječ 'med' na kraju rečenice kao imenica: 'Kupio je med.', ili je češće riječ o skraćenici, recimo 'med.' za 'medicina'.

Iza rednih brojeva LibreOffice ne ispravlja mala slova.

### Dodatne napomene
Unosi ove datoteke ne mogu sadržavati razmak, već se svaki dio takve višedijelne kratice mora unijeti posebno, što se ne mora uvijek biti razriješeno korisniku po volji.

Problem mogu biti kratice sastavljene od više dijelova kada se neke ili sve dijelove namjerno ne želi unijeti na popis. Tako se 'dr.' od 'doktor' ne nalazi na popisu iznimaka za ispravljanje maloga slova u veliko jer 'dr.' može značiti i 'i drugo' te se u tom značenju često nalazi na samome kraju rečenice.

S obzirom na prethodno, LibreOffice će 'dr. med. vet.' (doktor veterinarske medicine) ispraviti u 'dr. Med. Vet.' jer se 'dr.' i 'med.' ne nalaze na ovome popisu. Naravno, riječ iza 'vet.' neće biti automatski ispravljena iz maloga u veliko slovo jer se ta kratica nalazi na ovome popisu. Treba istražiti stoji li češće 'dr.' u značenju 'doktor' (znanosti) u sredini rečenice, ili češće 'dr.' (i drugo) stoji na kraju rečenice. To su dvije različite situacije koje traže dva različita rezultata, a radi se o ista tri znaka. U takvim će se situacijama morati raditi određeni kompromisi. Sada se 'dr. Vet. Med' ispravlja u 'dr. vet. med.' pomoću datoteke [DocumentList.xml](https://github.com/krunose/libo-acorr-hr/blob/master/DocumentList.xml).

Na ovom se popisu nalazi kratica 'sc.' Zanimljivo je to što 'dr. sc.' neće biti automatski ispravljeno u 'dr. Sc.' iako bi se to prema prethodno iznesenomu očekivalo. Prvi bi zaključak mogao biti da je tomu tako jer se kratica 'sc.' nalazi na popisu, međutim nije zbog toga. Ispravljanje se neće dogoditi ni s izmišljenim kraticama: 'gg. tt.' neće biti ispravljeno u 'gg. Tt.' bez obzira što se ni 'gg.' ni 'tt.' ne nalaze na ovome popisu. Ipak, napiše li se 'gg. ttt.', do zamjenjivanja će doći: 'gg. Ttt.' Dalje, zamjena se 'to' u 'To' neće dogoditi ako piše 'Jutros sam... to! Sunčano je.' (> Jutros sam... to! [...]), ali će do zamjene doći piše li 'Jutros! to!' (> Jutros! To!) Očito je važno koji znak prethodi i koliko znakova ima riječ koja slijedi, čak kada se i ne radi o kraticama.

Zaključak je da dvodjelne kratice u kojima drugi dio nije veći od dva slova, neće biti automatski ispravljene.

U ostalim situacijama treba naći mjeru. Može se ponešto ispraviti dopisivanjem pravila u datoteku DocumentList.xml. Svakako bi pomoglo poznavati točna pravila po kojima LibreOffice definira kraj rečenice, ali trebalo bi i znati za svaku takvu kraticu nalazi li se češće u sredini rečenice ili na kraju.

Kratice su sveučilišnih titula preuzete s http://narodne-novine.nn.hr/clanci/sluzbeni/2015_05_50_991.html; pristupljeno 17. ožujka 2017.

### Komentari uz problematične kratice

LibreOffice iza rednih brojeva ne ispravlja malo slovo u veliko. Tako je jer se redni brojevi pišu s točkom i u tom slučaju točka nije interpunkcijski nego pravopisni znak. Nevolja je u tome što se brojevi ponekad mogu naći i na samom kraju rečenice i tada točka stoji i kao pravopisni znak (redni broj) i kako interpunkcijski znak (kraj rečenice). Korisnik će sâm morati prvu riječ sljedeće rečenice pisati velikim slovom i tu se ne može puno napraviti.

#### g.
Odlučeno je da se na aktivni popis doda i kratica 'g.' iako se dosta često može nalaziti na kraju rečenice, često se i ne nalazi na kraju rečenice: 'U 2016. g. nismo uspjeli [...]', 'Bilo je to 2016.'. Kao i sa rednim brojevima, ni ovdje nije moguće računalu dati jednoznačne upute radi li se o kratici unutar rečenice ili kraju rečenice. Dostupni alati i mehanizmi nisu dostatni za razrješenje ambigviteta. Budući da je kratica dodana na popis, čak kada i bude na kraju rečenice, korisnik će sam morati prvu riječ sljedeće rečenice napisati velikim slovom, neće se provesti automatsko ispravljanje.

Uvrštavanjem se ove kratice na popis ne kvari funkcionalnost (u najgorem je slučaju ostala ista), dapače popravlja se u onim situacijama kada 'g.' stoji za 'gospodin' i tada se nikad ne nalazi na kraju rečenice nego se očekuje da iza nje slijedi ime.

Netko može pomisliti kako je ovaj mehanizam dobar za ispravljanje pisanja osobnih imena malim slovom iza kratice 'g.': g. marko > g. Marko. To nije dobro jer lomljenje jedne rečenice na dvije, koje to zapravo nisu, može prevariti provjeru gramatike i time onemogućiti provjeru reda riječi, pisanje zareza ili slično. O velikom i malom slovu treba brinuti provjera pravopisa i provjera gramatike.

Uzme li se u obzir da 'g.' stoji iza 'godina' i za 'gospodin', bolje ju je uključiti nego izostaviti, bez obzira što se u (rijetkim) slučajevima može nalaziti na kraju rečenice.

#### Kratice akademskih stupnjeva
Opisano u dijelu o datoteci SentenceExceptList.xml u [Dodatnim napomenama](https://github.com/krunose/libo-acorr-hr/blob/master/dokumentacija.md#dodatne-napomene).


## WordExceptList.xml
Datoteka WordExceptList.xml sadrži pravila za automatsko ispravljanje riječi koje su pogrešno napisane dvama velikim slovima na početku riječi: MArko > Marko.

Ima primjera, ponajviše kratica i skraćenica, koje i treba pisati dvama velikim slovima. Kratica 'OOo' stoji za 'OpenOffice.org'. Kada 'OOo' ne bi bio na ovom popisu, LibreOffice bi 'OOo' ispravljao u 'Ooo'.

Međutim, izgleda da ima i odstupanja. LibreOffice nije ispravljao kraticu 'MHz' u 'Mhz' bez obzira što se nije nalazila na ovom popisu i to je zato što se ta kratica nalazila u rječniku za provjeru pravopisa. Znači da se ovakvo automatsko ispravljanje provodi samo na riječima koje se ne nalaze u rječniku za provjeru pravopisa ili na ovom popisu. Za potvrdu teze može se izmisliti kratica 'MHt' i LibreOffice će je ispraviti u 'Mht', ali ako se kratica doda u rječnik za provjeru pravopisa, ili na ovaj popis, automatsko ispravljanje izostaje.

Na ovaj popis treba dodavati i kratice od dva velika početna slova koje su u kosim padežima pogrešno napisane bez spojnice između osnove i padežnoga nastavka. Kratica se za engl. izraza 'personal computer' piše dvama velikim slovima: PC. Genitivni se oblik piše sa spojnicom: PC-a. Međutim, često se može vidjeti pogrešno pisanje, bez spojnice: PCa. Problem je u tome što automatsko ispravljanje u 'PCa' vidi riječ od tri slova u kojoj su prva dva (pretpostavlja pogrešno) napisana velikim slovima i automatski se vrši ispravak u 'Pca'. Takav ispravak ne olakšava. Računalna će provjera pravopisa u pogrešci 'PCa' lako prepoznati 'PC-a' jer je razlika samo u jednom znaku – spojnici. Teže je nakon izvršenoga automatskog ispravka ispravka jer je razlika između 'Pca' i 'PC-a' u dvama znakovima – spojnici i malomu slovu c umjesto velikoga kako očekuje rječnik za provjeru pravopisa. Tako je 'Pca' na samo jedan znak razmaka od riječi 'Psa' (genitiv od 'pȁs') i provjera će pravopisa prvo ponudi tu riječ kao zamjenu umjesto da ponudi kraticu napisanu spojnicom. Zbog toga na ovaj popis iznimaka treba dodavati i pogreške poput 'PCa' kako se izbjeglo automatsko ispravljanje u situacijama kada bi takvo ispravljanje otežalo računalnu provjeru pravopisa. Naravno, treba voditi računa o tome da se sve kratice s ovoga popisa nalaze u rječniku za provjeru pravopisa (točno napisane).

Na ovom se popisu nalaze veće pogreške od nepisanja spojnice jer je važno da razlika između točnoga pisanja i pogreške bude što manja. Promjena drugoga slova takih kratica iz velikoga u malo samo produbljuje tu razliku. Namjerne su pogreške označene komentarom uz pojedinu kraticu (v. u datoteci WordExceptList.xml).


## dodano-na-documentlist.txt
U datoteci se navode unosi kojih nema na izvornom LibreOfficeovom popisu. Dodani su naknadno što je bilo potrebno radi lakšeg ispravljanja eventualnih pogrešaka i vođenja evidencije o tome što je i kada je dodano na popis.


## Umetanje-automatskih-ispravaka.md
U datoteci se opisuje postupak unošenja ovog popisa za automatsko ispravljanje u računalo. Često su pravila za automatsko ispravljanje iz [ovog repozitorija](https://github.com/krunose/libo-acor-hr) novija od onih koja se nalaze u izvornom kodu LibreOfficea.
